#version 330 core
#extension GL_ARB_shading_language_420pack : require

#define POSITION		0
#define COLOR			3
#define TEXCOORD		4
#define INSTANCE		7
#define FRAG_COLOR		0

#define DIFFUSE			0

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

layout(binding = DIFFUSE) uniform sampler2DArray Diffuse;

in block
{
	vec4 Color;
} In;

layout(location = FRAG_COLOR, index = 0) out vec4 Color;

void main()
{
	Color = In.Color;
}
