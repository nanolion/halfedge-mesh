#pragma once

#include "glm/glm.hpp"

struct Vertex
{
	Vertex( uint id, vec3 position, vec4 color ) : id( id ), position( position ), color( color ) /*, edge( nullptr )*/
	{

	};

	Vertex( uint id, vec3 position ) : id( id ), position( position ), color( vec4( 1.0f, 1.0f, 1.0f, 1.0f ) )/*, edge( nullptr )*/
	{

	};

	const uint id;
	vec3 position;
	vec4 color;
	float light = 0.5f;
	bool initialized = false;
};