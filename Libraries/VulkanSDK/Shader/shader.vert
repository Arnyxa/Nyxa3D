#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uniform_buffer_object
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

layout(location = 0) in vec2 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

out gl_PerVertex 
{ vec4 gl_Position; };

void main() 
{
    gl_Position = ubo.projection 
                * ubo.view 
                * ubo.model
                * vec4(in_position, 0.0, 1.0);
    frag_color = in_color;
}