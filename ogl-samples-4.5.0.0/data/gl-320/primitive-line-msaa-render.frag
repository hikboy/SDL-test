#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

in block
{
	vec2 Texcoord;
} In;

out vec4 Color;

void main()
{
	Color = vec4(1.0, 0.5, 0.0, 1.0);
}
