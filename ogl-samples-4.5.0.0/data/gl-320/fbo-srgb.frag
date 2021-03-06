#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;
layout(origin_upper_left) in vec4 gl_FragCoord;

uniform sampler2D Diffuse;

in block
{
	vec2 Texcoord;
} In;

out vec4 Color;

void main()
{
	Color = texture(Diffuse, In.Texcoord);
}
