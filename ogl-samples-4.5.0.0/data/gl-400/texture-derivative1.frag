#version 150 core

precision highp float;
precision highp int;
layout(std140, column_major) uniform;

uniform sampler2D Diffuse;
uniform float FramebufferSize;

in vec4 gl_FragCoord;
out vec4 Color;

void main()
{
	vec2 TextureSize = vec2(textureSize(Diffuse, 0)) * FramebufferSize;

	Color = texture(Diffuse, gl_FragCoord.xy / TextureSize);
	//Color = texelFetch(Diffuse, ivec2(gl_FragCoord.xy), 0);
}
