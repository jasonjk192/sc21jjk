#version 450

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform sampler2D texture_diffuse1;

uniform float shininess;
uniform float metalness;
uniform vec4 specularColor;
uniform vec4 emissiveColor;

vec3 ambientLight = vec3(0.1);
float pi = 3.1415926535897932384626433832795028;

float clampedDot(vec3 a, vec3 b)
{
    return max(0,dot(a,b));
}

float D(vec3 n, vec3 h)
{
    return (shininess+2)/(2.0*pi) * pow(clampedDot(n,h),shininess);
}

float G(vec3 n, vec3 l, vec3 v, vec3 h)
{
    return min(1.0, min( 2.0*(clampedDot(n,h)*clampedDot(n,v)/dot(v,h)), 2.0*(clampedDot(n,h)*clampedDot(n,l)/dot(v,h)) ));
}

vec3 F(vec3 h, vec3 v)
{
    vec3 F0 = (1.0-metalness)*vec3(.04,.04,.04) + metalness*texture(texture_diffuse1, TexCoord).rgb;
    return F0 + (1.0-F0)*pow(1.0-dot(h,v), 5.0);
}

vec3 Ldiffuse(vec3 h, vec3 v)
{
    return (texture(texture_diffuse1, TexCoord).rgb/pi * (vec3(1)-F(h,v)))*(1-metalness);
}

void main()
{
    vec4 diffuseColor = texture(texture_diffuse1, TexCoord);
    vec3 totalColor = emissiveColor.xyz + ambientLight*diffuseColor.rgb;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir= normalize(lightPos - FragPos);
    vec3 h = normalize(viewDir + lightDir);

    vec3 fr = Ldiffuse(h, viewDir) * clampedDot(Normal,lightDir) + (D(Normal, h) * F(h, viewDir) * G(Normal, lightDir, viewDir, h)) / max(0.1f,4 * clampedDot(Normal,viewDir));
    //vec3 fr = vec3(D(Normal, h) * F(h, viewDir) * G(Normal, lightDir, viewDir, h));
    totalColor += fr * lightColor;

    FragColor = vec4( totalColor, 1.0 );
}