#version 430

layout(local_size_x=256) in;
layout(location = 0) uniform float dt;
layout(std430, binding=0) buffer position_block
{
	vec3 positions[];
};

layout(std430, binding=1) buffer velocity_block
{
	vec3 velocities[];
};

void main()
{
	int index = int(gl_GlobalInvocationID);

	vec3 currentPos = positions[index];
	currentPos += velocities[index]*dt;
	positions[index] = currentPos;
}