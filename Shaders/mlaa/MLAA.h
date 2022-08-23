/**
 * Copyright (C) 2010 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2010 Belen Masia (bmasia@unizar.es)
 * Copyright (C) 2010 Jose I. Echevarria (joseignacioechevarria@gmail.com)
 * Copyright (C) 2010 Fernando Navarro (fernandn@microsoft.com)
 * Copyright (C) 2010 Diego Gutierrez (diegog@unizar.es)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the following statement:
 *
 *       "Uses Jimenez's MLAA. Copyright (C) 2010 by Jorge Jimenez, Belen Masia,
 *        Jose I. Echevarria, Fernando Navarro and Diego Gutierrez."
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holders.
 */

// Porting functions
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4

#ifndef MLAA_ONLY_COMPILE_VS
#define MLAA_ONLY_COMPILE_VS 0
#endif
#ifndef MLAA_ONLY_COMPILE_PS
#define MLAA_ONLY_COMPILE_PS 0
#endif

// Just for checking syntax at compile time
#if !defined(PIXEL_SIZE)
#define PIXEL_SIZE float2(1.0 / 1280.0, 1.0 / 720.0)
#define MAX_SEARCH_STEPS 8
#define MAX_DISTANCE 32
#endif

/**
 * Here we have an interesting define. In the last pass we make usage of
 * bilinear filtering to avoid some lerps; however, bilinear filtering
 * in DX9, under DX9 hardware (but not in DX9 code running on DX10 hardware)
 * is done in gamma space, which gives sustantially worser results. So, this
 * flag allows to avoid the bilinear filter trick, changing it with some
 * software lerps.
 *
 * So, to summarize, it is safe to use the bilinear filter trick when you are
 * using DX10 hardware on DX9. However, for the best results when using DX9
 * hardware, it is recommended comment this line.
 */

#define BILINEAR_FILTER_TRICK

/**
 *  V E R T E X   S H A D E R S
 */

#if MLAA_ONLY_COMPILE_PS == 0

void PassThroughVS(inout float4 position,
inout float2 texcoord) {
}

void OffsetVS(inout float4 position,
inout float2 texcoord,
out float4 offset[2]) {
offset[0] = texcoord.xyxy + PIXEL_SIZE.xyxy * float4(-1.0, 0.0, 0.0, -1.0);
offset[1] = texcoord.xyxy + PIXEL_SIZE.xyxy * float4( 1.0, 0.0, 0.0,  1.0);
}

#endif // MLAA_ONLY_COMPILE_PS == 0


#if MLAA_ONLY_COMPILE_VS == 0

//-----------------------------------------------------------------------------
// Misc functions
//    /**
//     * Typical Multiply-Add operation to ease translation to assembly code.
//     */

float4 mad(float4 m, float4 a, float4 b) {
#if defined(XBOX)
    float4 result;
    asm {
        mad result, m, a, b
    };
    return result;
#else
    return m * a + b;
#endif
}


//    /**
//     * This one just returns the first level of a mip map chain, which allow us to
//     * avoid the nasty ddx/ddy warnings, even improving the performance a little
//     * bit.
//     */

float4 tex2Dlevel0(sampler2D map, float2 texcoord) {
    //return textureLod(map, float4(texcoord, 0.0, 0.0));
    return textureLod(map, texcoord, 0);
}


//    /**
//     * Same as above, this eases translation to assembly code;
//     */

float4 tex2Doffset(sampler2D map, float2 texcoord, float2 offset) {
#if defined(XBOX) && MAX_SEARCH_STEPS < 6
    float4 result;
    float x = offset.x;
    float y = offset.y;
    asm {
        tfetch2D result, texcoord, map, OffsetX = x, OffsetY = y
    };
    return result;
#else
    return tex2Dlevel0(map, texcoord + PIXEL_SIZE * offset);
#endif
}


//    /**
//     * Ok, we have the distance and both crossing edges, can you please return
//     * the float2 blending weights?
//     */

float2 Area(float2 distance, float e1, float e2, sampler2D areaMap) {
    // * By dividing by areaSize - 1.0 below we are implicitely offsetting to
    //   always fall inside of a pixel
    // * Rounding prevents bilinear access precision problems
    float areaSize = MAX_DISTANCE * 5.0;
    float2 pixcoord = MAX_DISTANCE * round(4.0 * float2(e1, e2)) + distance;
    float2 texcoord = pixcoord / (areaSize - 1.0);
    return tex2Dlevel0(areaMap, texcoord).ra;
}

//    /**
//     *  1 S T   P A S S   ~   C O L O R   V E R S I O N
//     */

float4 ColorEdgeDetectionPS(float2 texcoord, float4 offset[2], sampler2D colorMapG, float threshold)
{
    float3 weights = float3(0.2126,0.7152, 0.0722); // These ones are from the ITU-R Recommendation BT. 709
//    /**
//     * Luma calculation requires gamma-corrected colors:
//     */
    float L = dot(texture(colorMapG, texcoord).rgb, weights);
    float Lleft = dot(texture(colorMapG, offset[0].xy).rgb, weights);
    float Ltop = dot(texture(colorMapG, offset[0].zw).rgb, weights);
    float Lright = dot(texture(colorMapG, offset[1].xy).rgb, weights);
    float Lbottom = dot(texture(colorMapG, offset[1].zw).rgb, weights);

    float4 delta = abs(float4(L) - float4(Lleft, Ltop, Lright, Lbottom));
    float4 edges = step(float4(threshold), delta);

//    if (dot(edges, float4(1.0)) == 0.0)
//        discard;

    return edges;
}


//    /**
//     *  1 S T   P A S S   ~   D E P T H   V E R S I O N
//     */

/*
float4 DepthEdgeDetectionPS(float2 texcoord,float4 offset[2], sampler2D depthMap) {
    float D = texture(depthMap, texcoord).r;
    float Dleft = texture(depthMap, offset[0].xy).r;
    float Dtop  = texture(depthMap, offset[0].zw).r;
    float Dright = texture(depthMap, offset[1].xy).r;
    float Dbottom = texture(depthMap, offset[1].zw).r;

    float4 delta = abs(float4(D) - float4(Dleft, Dtop, Dright, Dbottom));
    float4 edges = step(float4(threshold) / 10.0, delta); // Dividing by 10 give us results similar to the color-based detection.

    if (dot(edges, float4(1.0)) == 0.0)
    discard;

    return edges;
}
*/


//    /**
//     * Search functions for the 2nd pass.
//     */

/*
float SearchXLeft(float2 texcoord) {
    // We compare with 0.9 to prevent bilinear access precision problems.
    float i;
    float e = 0.0;
    for (i = -1.5; i > -2.0 * MAX_SEARCH_STEPS; i -= 2.0) {
        e = tex2Doffset(edgesMapL, texcoord, float2(i, 0.0)).g;
        if (e < 0.9) break;
    }
    return max(i + 1.5 - 2.0 * e, -2.0 * MAX_SEARCH_STEPS);
}

float SearchXRight(float2 texcoord) {
    float i;
    float e = 0.0;
    for (i = 1.5; i < 2.0 * MAX_SEARCH_STEPS; i += 2.0) {
        e = tex2Doffset(edgesMapL, texcoord, float2(i, 0.0)).g;
        if (e < 0.9) break;
    }
    return min(i - 1.5 + 2.0 * e, 2.0 * MAX_SEARCH_STEPS);
}

float SearchYUp(float2 texcoord) {
    float i;
    float e = 0.0;
    for (i = -1.5; i > -2.0 * MAX_SEARCH_STEPS; i -= 2.0) {
        e = tex2Doffset(edgesMapL, texcoord, float2(i, 0.0).yx).r;
        if (e < 0.9) break;
    }
    return max(i + 1.5 - 2.0 * e, -2.0 * MAX_SEARCH_STEPS);
}

float SearchYDown(float2 texcoord) {
    float i;
    float e = 0.0;
    for (i = 1.5; i < 2.0 * MAX_SEARCH_STEPS; i += 2.0) {
        e = tex2Doffset(edgesMapL, texcoord, float2(i, 0.0).yx).r;
        if (e < 0.9) break;
    }
    return min(i - 1.5 + 2.0 * e, 2.0 * MAX_SEARCH_STEPS);
}
*/


//    /**
//     *  2 N D   P A S S
//     */

/*
float4 BlendWeightCalculationPS(float2 texcoord, sampler2D edgesMap, sampler2D areaMap){
    float4 areas = float4(0.0);

    float2 e = texture(edgesMap, texcoord).rg;

    if (e.g > 0.0) { // Edge at north

        // Search distances to the left and to the right:
        float2 d = float2(SearchXLeft(texcoord), SearchXRight(texcoord));

        // Now fetch the crossing edges. Instead of sampling between edgels, we
        // sample at -0.25, to be able to discern what value has each edgel:
        float4 coords = mad(float4(d.x, -0.25, d.y + 1.0, -0.25),PIXEL_SIZE.xyxy, texcoord.xyxy);
        float e1 = tex2Dlevel0(edgesMapL, coords.xy).r;
        float e2 = tex2Dlevel0(edgesMapL, coords.zw).r;

        // Ok, we know how this pattern looks like, now it is time for getting
        // the actual area:
        areas.rg = Area(abs(d), e1, e2, areaMap);
    }

    if (e.r > 0.0) { // Edge at west

        // Search distances to the top and to the bottom:
        float2 d = float2(SearchYUp(texcoord), SearchYDown(texcoord));

        // Now fetch the crossing edges (yet again):
        float4 coords = mad(float4(-0.25, d.x, -0.25, d.y + 1.0),
                            PIXEL_SIZE.xyxy, texcoord.xyxy);
        float e1 = tex2Dlevel0(edgesMapL, coords.xy).g;
        float e2 = tex2Dlevel0(edgesMapL, coords.zw).g;

        // Get the area for this direction:
        areas.ba = Area(abs(d), e1, e2);
    }

    return areas;
}
*/


//    /**
//     *  3 R D   P A S S
//     */

/*
float4 NeighborhoodBlendingPS(float2 texcoord,float4 offset[2], sampler2D colorMapL, sampler2D blendMap){
    // Fetch the blending weights for current pixel:
    float4 topLeft = texture(blendMap, texcoord);
    float bottom = texture(blendMap, offset[1].zw).g;
    float right = texture(blendMap, offset[1].xy).a;
    float4 a = float4(topLeft.r, bottom, topLeft.b, right);

    // Up to 4 lines can be crossing a pixel (one in each edge). So, we perform
    // a weighted average, where the weight of each line is 'a' cubed, which
    // favors blending and works well in practice.
    float4 w = a * a * a;

    // There is some blending weight with a value greater than 0.0?
    float sum = dot(w, 1.0);
    if (sum < 1e-5)
    discard;

    float4 color = 0.0;

    // Add the contributions of the possible 4 lines that can cross this pixel:
    #ifdef BILINEAR_FILTER_TRICK
    float4 coords = mad(float4( 0.0, -a.r, 0.0,  a.g), PIXEL_SIZE.yyyy, texcoord.xyxy);
    color = mad(texture(colorMapL, coords.xy), w.r, color);
    color = mad(texture(colorMapL, coords.zw), w.g, color);

    coords = mad(float4(-a.b,  0.0, a.a,  0.0), PIXEL_SIZE.xxxx, texcoord.xyxy);
    color = mad(texture(colorMapL, coords.xy), w.b, color);
    color = mad(texture(colorMapL, coords.zw), w.a, color);
    #else
    float4 C = texture(colorMap, texcoord);
            float4 Cleft = texture(colorMap, offset[0].xy);
            float4 Ctop = texture(colorMap, offset[0].zw);
            float4 Cright = texture(colorMap, offset[1].xy);
            float4 Cbottom = texture(colorMap, offset[1].zw);
            color = mad(lerp(C, Ctop, a.r), w.r, color);
            color = mad(lerp(C, Cbottom, a.g), w.g, color);
            color = mad(lerp(C, Cleft, a.b), w.b, color);
            color = mad(lerp(C, Cright, a.a), w.a, color);
    #endif

    // Normalize the resulting color and we are finished!
    return color / sum;
}
*/

#endif // MLAA_ONLY_COMPILE_VS == 0
