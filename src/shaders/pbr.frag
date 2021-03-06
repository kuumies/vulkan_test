/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   PBR fragment shader.
 * -------------------------------------------------------------------------- */

#version 450

// -----------------------------------------------------------------------------
// Uniform light

layout(binding = 1) uniform Light
{
    vec4 worldDir;  // Direction of light in world space.
    vec4 intensity; // Itensity of light (values can be over 1.0)

} light;

// -----------------------------------------------------------------------------
// Params

layout(binding = 2) uniform PbrParams
{
    vec4 camPos;
    vec4 albedo;
    float metallic;
    float roughness;
    float ao;

} pbrParams;

// -----------------------------------------------------------------------------
// Material maps

layout(binding = 3)  uniform sampler2D ambientOcclusionMap;
layout(binding = 4)  uniform sampler2D baseColorMap;
layout(binding = 5)  uniform sampler2D heightMap;
layout(binding = 6)  uniform sampler2D metallicMap;
layout(binding = 7)  uniform sampler2D normalMap;
layout(binding = 8)  uniform sampler2D roughnessMap;

// -----------------------------------------------------------------------------
// Generated maps

layout(binding = 9)  uniform samplerCube irradianceMap;
layout(binding = 10)  uniform samplerCube prefilteredMap;
layout(binding = 11) uniform sampler2D brdfLutMap;
layout(binding = 12) uniform sampler2D shadowMap;

// -----------------------------------------------------------------------------
// Vertex shader outputs

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 worldNormal;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in vec4 lightPos;
layout(location = 4) in mat3 tbn;

// -----------------------------------------------------------------------------
// Fragment shader outputs

layout(location = 0) out vec4 outColor;

// -----------------------------------------------------------------------------
// Constants

const float PI                 = 3.14159;
const float MAX_REFLECTION_LOD = 4.0;

// -----------------------------------------------------------------------------
// Cook-Torrance specular BRDF (GGX) normal distribution function.

float brdfNormalDistributionGGX(float nDotH, float a)
{
    float nDotH2 = nDotH * nDotH;
    float a2     = a * a;
    float q      = (nDotH2 * (a2 -1.0) + 1.0);
    return a2 / (PI * q * q);
}

// -----------------------------------------------------------------------------
// Cook-Torrance specular BRDF (GGX) geometry function.

float brdfGeometryGGX(float nDotV, float nDotL, float k)
{
    float ggx1 = nDotV / (nDotV * (1.0 - k) + k);
    float ggx2 = nDotL / (nDotL * (1.0 - k) + k);

    return ggx1 * ggx2;
}

// -----------------------------------------------------------------------------
// Diffuse / specular ration.

vec3 brdfFresnel(float nDotV, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - nDotV, 5.0);
}

// -----------------------------------------------------------------------------
//

vec3 brdfFresnelRoughness(float nDotV, vec3 f0, float roughness)
{
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - nDotV, 5.0);
}

// -----------------------------------------------------------------------------
// Offsets the texture coordinates based on the height map.

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height_scale = 0.1;

    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

     float layerDepth = 1.0 / numLayers;
     float currentLayerDepth = 0.0;
     vec2 P = viewDir.xy * height_scale;
     vec2 deltaTexCoords = P / numLayers;

     vec2  currentTexCoords     = texCoords;
     float currentDepthMapValue = 1.0 - texture(heightMap, currentTexCoords).r;

     while(currentLayerDepth < currentDepthMapValue)
     {
         currentTexCoords -= deltaTexCoords;
         currentDepthMapValue = 1.0 - texture(heightMap, currentTexCoords).r;
         currentLayerDepth += layerDepth;
     }

     vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

     float afterDepth  = currentDepthMapValue - currentLayerDepth;
     float beforeDepth = 1.0 - texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;

     float weight = afterDepth / (afterDepth - beforeDepth);
     vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

//     if(finalTexCoords.x > 1.0 || finalTexCoords.y > 1.0 || finalTexCoords.x < 0.0 || finalTexCoords.y < 0.0)
//         return texCoord;

     return finalTexCoords;
}

float shadowFactor(float epsilon)
{
    // Transform the vertex from the light points of view in UV space
    vec3 ndc = lightPos.xyz / lightPos.w;
    vec3 uv = ndc;

    // Vertex distance from the light point of view in UV space
    float z = uv.z;

    // The closest pixel from the light point of view in UV space.
    float depth = texture(shadowMap, uv.xy).r;

    return smoothstep(z - epsilon, z, depth);
}

/* ---------------------------------------------------------------- */

void main()
{
    // Calculate vectors.
    vec3 v = normalize(pbrParams.camPos.xyz - worldPos);
    vec3 l = normalize(-light.worldDir.xyz);
    vec3 h = normalize(l + v);
    vec3 n = normalize(worldNormal);

    // Offset texture coordinates if height map exits
    vec2 tc = texCoord;
    if (textureSize(heightMap, 1).x > 1)
    {
        vec3 tangent = normalize(transpose(tbn) * v);
        tc = parallaxMapping(tc, tangent);
    }

    // Sample maps
    float metallic = pbrParams.metallic;
    if (textureSize(metallicMap, 1).x > 1)
        metallic  = texture(metallicMap, tc).r;

    float roughness = pbrParams.roughness;
    if (textureSize(roughnessMap, 1).x > 1)
        roughness = texture(roughnessMap, tc).r;
    vec3 albedo = pbrParams.albedo.rgb;
    if (textureSize(baseColorMap, 1).x > 1)
        albedo = texture(baseColorMap, tc).rgb;

    // Use ambient occlusion from map if available
    float ao = pbrParams.ao;
    if (textureSize(ambientOcclusionMap, 1).x > 1)
        ao = texture(ambientOcclusionMap, tc).r;

    // Use normal form map if available
    if (textureSize(normalMap, 1).x > 1)
    {
        n = texture(normalMap, tc).rgb;
        n = normalize(n * 2.0 - 1.0);
        n = tbn * n;
        n = normalize(n);
    }

    // Vector angles
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = max(dot(n, v), 0.0);
    float nDotH = max(dot(n, h), 0.0);

    // Base reflectivity
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);

    // Specular BRDF of light.
    float ndf = brdfNormalDistributionGGX(nDotH, roughness * roughness);
    float g   = brdfGeometryGGX(nDotV, nDotL, roughness);
    vec3 f    = brdfFresnel(nDotL, f0);

    // Reflection/refraction ratio
    vec3 kS = f;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // Calc. light diffuse and specular radiance
    vec3 radianceDiffuse  = kD * albedo / PI;

    // Mix shadow
    vec3 shadowColor = vec3(0.01, 0.01, 0.01);
    float shadow = shadowFactor(0.005);

    vec3 radianceSpecular = (ndf * g * f )/ max(4.0 * nDotV * nDotL, 0.001);

    // Calc. light radiance
    vec3 radiance = (radianceDiffuse + radianceSpecular) * light.intensity.rgb * nDotL;
    radiance = mix(shadowColor, radiance, shadow);

    // Use precalculated diffuse light irradiance
    vec3 irradianceDiffuse  = texture(irradianceMap, n).rgb * albedo;


    // Use precalculated specular light irradiance
    vec3 r = reflect(-v, n);
    r.z = -r.z;
    vec3 fr = brdfFresnelRoughness(nDotV, f0, roughness);
    //vec3 prefilteredColor = textureLod(prefilteredMap, r, roughness * MAX_REFLECTION_LOD).rgb;
    vec3 prefilteredColor = texture(prefilteredMap, r).rgb;
    vec2 envBRDF  = texture(brdfLutMap, vec2(nDotV, roughness)).rg;
    vec3 irradianceSpecular = prefilteredColor * (fr * envBRDF.x + envBRDF.y);

    vec3 irradiance = (kD * irradianceDiffuse + irradianceSpecular) * ao;

    outColor.rgb = radiance + irradiance;

    // reinhard tone mapping
    outColor = outColor / (outColor + vec4(1.0));
    outColor.a = 1.0;
}
