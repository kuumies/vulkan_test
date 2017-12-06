/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Diffuse fragment shader.
 * ---------------------------------------------------------------- */

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSampler, uv);
    if (outColor.g == 0 && outColor.b == 0)
        outColor = vec4(outColor.r, outColor.r, outColor.r, outColor.a);
}
