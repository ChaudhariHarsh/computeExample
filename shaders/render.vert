#version 430

layout(location=0) in vec3 pos;
layout(location=0) uniform mat4 MVP;
//layout(location=1) uniform mat4 projection;

void main()
{
	gl_Position = MVP*vec4(pos,1);
}
