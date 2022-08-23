#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform float shininess;
uniform float metalness;
uniform vec4 specularColor;
uniform vec4 emissiveColor;

uniform sampler2D texture_diffuse1;

vec3 ambientLight = vec3(0.1);
float pi = 3.1415926535897932384626433832795028;

float clampedDot(vec3 a, vec3 b)
{
    return max(0,dot(a,b));
}

vec3 Blinn2()
{
    vec3 norm = normalize(Normal);
    vec4 diffuseColor = texture(texture_diffuse1, TexCoord);
    vec3 totalColor = emissiveColor.xyz + ambientLight;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir= normalize(lightPos - FragPos);
    vec3 h = normalize(viewDir + lightDir);
    totalColor += (diffuseColor.xyz/pi + (shininess+2.f)/8.f * pow(clampedDot(norm, h),shininess) * specularColor.xyz)*clampedDot(norm, lightDir)* lightColor.xyz;
    return totalColor;
}

vec3 Blinn1()
{
    vec4 oColor = texture(texture_diffuse1, TexCoord);
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    float specularStrength = shininess;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    return (ambient + diffuse + specular) * oColor.rgb;
}

void main()
{
    vec3 result = Blinn1();
    FragColor = vec4(result, 1.0);
    //FragColor = texture(texture_diffuse1, TexCoord);
}