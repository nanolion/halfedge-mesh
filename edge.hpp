#pragma once

#include "glm/glm.hpp"

struct Edge
{
	Edge( uint id, VertexHandle vertex ) : id( id ), vertex( vertex ), next( nullptr ), opposing( nullptr ), face( nullptr ), texcoord( 0.0f, 0.0f ), barycenter( 0.0f, 0.0f, 0.0f )
	{

	};

	const uint id;
	VertexHandle vertex;
	EdgeHandle next;
	EdgeHandle opposing;
	FaceHandle face;
	vec2 texcoord;
	vec3 barycenter;
	bool initialized = false;
};