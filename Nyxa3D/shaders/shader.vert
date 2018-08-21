#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex {
	vec4 gl_Position;
};

layout(location = 0) out vec3 frag_color;

const float max = 0.75;
const float min = 0.4;

vec2 positions[3] = vec2[]
(
    vec2(0.0, -max),
    vec2(max, min),
    vec2(-max, min),

    // vec2(max, -min),
    // vec2(0.0, max),
    // vec2(-max, -min)
);

vec3 colors[3] = vec3[]
(
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() 
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    frag_color = colors[gl_VertexIndex % 3];
}