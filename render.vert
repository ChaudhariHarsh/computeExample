#version 430

layout(location=0) in vec3 pos;
layout(location=0) uniform mat4 view;
layout(location=1) uniform mat4 projection;

void main()
{
	gl_Position = projection*view*vec4(pos,1);
}
