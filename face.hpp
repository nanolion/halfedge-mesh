#pragma once

#include "glm/glm.hpp"

struct Face
{
	Face( uint id, EdgeHandle edge ) : id( id ), edge( edge ), color( vec4( 1.0f, 1.0f, 1.0f, 1.0f ) )
	{

	};

	const uint id;
	EdgeHandle edge;
	vec3 normal;
	vec4 color;
	bool initialized = false;
};