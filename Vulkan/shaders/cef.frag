#version 450

layout(binding = 0) uniform sampler2D samplerCEF;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outFragcolor;

void main()
{
	vec4 cef = texture(samplerCEF, gl_FragCoord.xy / vec2(1024,768));
	outFragcolor = vec4(cef.r, cef.g, cef.b, cef.a);
}