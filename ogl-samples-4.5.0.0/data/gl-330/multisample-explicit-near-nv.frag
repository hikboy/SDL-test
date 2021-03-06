#version 330 core
#extension GL_NV_explicit_multisample : require

#define FRAG_COLOR		0

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

uniform samplerRenderbuffer Diffuse;

in block
{
	vec2 Texcoord;
} In;

layout(location = FRAG_COLOR, index = 0) out vec4 Color;

void main()
{
	// integer UV coordinates, needed for fetching multisampled texture
	ivec2 Texcoord = ivec2(textureSizeRenderbuffer(Diffuse) * In.Texcoord);

	Color = texelFetchRenderbuffer(Diffuse, Texcoord, 0);
}
