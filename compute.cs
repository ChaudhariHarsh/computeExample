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
	//why take only 'x' dimensions?
	int N = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x);
	int index = int(gl_GlobalInvocationID);

	vec3 our_position = positions[index].xyz;
	vec3 our_velocity = velocities[index].xyz;

	vec3 acceleration = vec3(0,0,0);
	for(int i=0;i<N;++i)
	{
		vec3 otherParticlePosition = positions[i];
		vec3 diff = otherParticlePosition - our_position;
		float invDist = 1.0/(length(diff) + 0.001);
		//add 0.001 so that if distance between 2 particles 
		//becomes 0 acceleration doesn't go to infinity
		acceleration += diff*0.1*invDist*invDist;
	}
	velocities[index] = vec3(our_velocity+ dt*acceleration);
}