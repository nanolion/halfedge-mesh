#include "core/mesh/LinkedMesh.hpp"

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::LinkedMesh()
{

}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::~LinkedMesh()
{

}

/// ////////////////////////////////////////////////////////////////////////////
void LinkedMesh::set_color( const FaceHandle face, const glm::vec4 & color )
{
    const auto first_edge = face->edge;
    auto edge = first_edge;

    assert( edge->vertex );

    do
    {
        edge->vertex->color = color;
    }
    while( ( edge = edge->next ) != first_edge );
}

/// ////////////////////////////////////////////////////////////////////////////
void LinkedMesh::set_color( const unsigned int face_index, const vec4 & color )
{
    assert( face_index < m_faces.size() );
    m_faces[face_index]->color = color;
}

/// ////////////////////////////////////////////////////////////////////////////
void LinkedMesh::set_texcoord( const unsigned int face_index, const std::vector<vec2> & new_texcoords )
{
    assert( face_index < m_faces.size() );
    const auto first_edge = m_faces[face_index]->edge;
    auto edge = first_edge;

    int edge_count = 0;

    do
    {
        edge->texcoord = new_texcoords[edge_count++];
    }
    while( ( edge = edge->next ) != first_edge );

    /// make sure the face has exactly as many vertices as new_texcoords were passed in
    assert( edge_count == new_texcoords.size() );
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::VertexHandle LinkedMesh::add_vertex( const vec3 & position, const vec4 & color )
{
    VertexHandle new_vertex;

    if( !m_free_vertices.empty() )
    {
        new_vertex = m_vertices[m_free_vertices.back()].get();
        m_free_vertices.pop_back();
        new_vertex->position = position;
        new_vertex->color = color;
    }
    else
    {
        m_vertices.emplace_back( make_unique<Vertex>( m_vertices.size(), position, color ) );
        new_vertex = m_vertices.back().get();
    }

    assert( new_vertex->initialized == false );
    new_vertex->initialized = true;

    return new_vertex;
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::EdgeHandle LinkedMesh::add_halfedge( const VertexHandle vertex )
{
    EdgeHandle new_edge;
    if( !m_free_edges.empty() )
    {
        /// use a free edge if possible
        new_edge = m_edges[m_free_edges.back()].get();
        m_free_edges.pop_back();
        new_edge->vertex = vertex;
    }
    else
    {
        /// otherwise allocate a new edge
        m_edges.emplace_back( make_unique<Edge>( m_edges.size(), vertex ) );
        new_edge = m_edges.back().get();
    }

    assert( new_edge->initialized == false );
    new_edge->initialized = true;

    return new_edge;
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::FaceHandle LinkedMesh::add_face( const vec3 & p0, const vec3 & p1, const vec3 & p2, const vec4 & color )
{
    EdgeLoop edge_loop;
    edge_loop.emplace_back( add_halfedge( add_vertex( p0, color ) ) );
    edge_loop.emplace_back( add_halfedge( add_vertex( p1, color ) ) );
    edge_loop.emplace_back( add_halfedge( add_vertex( p2, color ) ) );
    return add_face( edge_loop );
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::FaceHandle LinkedMesh::add_face( const vec3 & p0, const vec3 & p1, const vec3 & p2, const vec3 & p3, const vec4 & color )
{
    EdgeLoop edge_loop;
    edge_loop.emplace_back( add_halfedge( add_vertex( p0, color ) ) );
    edge_loop.emplace_back( add_halfedge( add_vertex( p1, color ) ) );
    edge_loop.emplace_back( add_halfedge( add_vertex( p2, color ) ) );
    edge_loop.emplace_back( add_halfedge( add_vertex( p3, color ) ) );
    return add_face( edge_loop );
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::FaceHandle LinkedMesh::add_face( EdgeLoop && edges )
{
    return add_face( edges );
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::FaceHandle LinkedMesh::add_face( EdgeLoop & edges )
{
    assert( edges.size() > 2 );

    FaceHandle new_face;

    if( !m_free_faces.empty() )
    {
        /// if there is a free face on the stack reuse it
        new_face = m_faces[m_free_faces.back()].get();
        //assert( new_face->edge == nullptr );
        //assert( edges.front() != nullptr );
        new_face->edge = edges.front();
        m_free_faces.pop_back();
    }
    else
    {
        /// when there is no free faces stacked allocate a new
        m_faces.emplace_back( make_unique<Face>( m_faces.size(), edges.front() ) );
        new_face = m_faces.back().get();
    }

    assert( new_face->initialized == false );
    new_face->initialized = true;

    /// make edgeloop
    for( unsigned int i = 0; i < edges.size(); ++i )
    {
        if( edges[i]->next != nullptr )
        {
            std::cout << "error index:" << i << std::endl;
            throw 1;
        }

        //assert( edges[i]->next == nullptr );

        edges[i]->next = edges[( i+1 )%edges.size()];
        edges[i]->face = new_face;
        switch( i )
        {
        case 0:
        {
            edges[i]->texcoord = vec2(0.0, 0.0);
            break;
        }

        case 1:
        {
            edges[i]->texcoord = vec2(1.0, 0.0);
            break;
        }

        case 2:
        {
            edges[i]->texcoord = vec2(1.0, 1.0);
            break;
        }

        case 3:
        {
            edges[i]->texcoord = vec2(0.0, 1.0);
            break;
        }
        }

    }

    auto compute_triangle_normal = []( const vec3 & p0, const vec3 & p1, const vec3 p2 )
    {
        /// evaluate normal from three unique positions
        return glm::normalize( glm::cross( ( p0 - p1 ), ( p0 - p2 ) ) );
    };

    new_face->normal = compute_triangle_normal( edges[0]->vertex->position, edges[1]->vertex->position, edges[2]->vertex->position );
    
    new_face->color = edges[0]->vertex->color;

    return new_face;
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::FaceHandle LinkedMesh::bridge_edges( const EdgeHandle edge_left, const EdgeHandle edge_right )
{
    assert( edge_left->next && edge_right->next );
    assert( edge_left->opposing == nullptr && edge_right->opposing == nullptr );

    std::vector<EdgeHandle> edge_loop;

    VertexHandle vertex[] =
    {
        edge_left->vertex,
        edge_left->next->vertex,
        edge_right->vertex,
        edge_right->next->vertex
    };

    /*
        both vertices   one vertex is       both vertices
        are different   different           different
        1...2           1 = 2   1---2       1 = 2
        |   |           |\      |  /        |
        |   |           | \     | /         |
        |   |           |  \    |/          |
        0...3           0---3   0 = 3       0 = 3

        create 2        create 1            create 0
        new edges       new edges           new edges
    */

    if( vertex[0] == vertex[3] && vertex[1] == vertex[2] )
    {
        link_edges( edge_left, edge_right );
        return nullptr;
    }
    else
    {
        edge_loop.emplace_back( add_halfedge( vertex[1] ) );
        link_edges( edge_loop.back(), edge_left );
        edge_loop.emplace_back( add_halfedge( vertex[0] ) );
        if( vertex[0] != vertex[3] )
        {
            edge_loop.emplace_back( add_halfedge( vertex[3] ) );
        }
        link_edges( edge_loop.back(), edge_right );
        if( vertex[1] != vertex[2] )
        {
            edge_loop.emplace_back( add_halfedge( vertex[2] ) );
        }
    }
    return add_face( edge_loop );
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::FaceHandle LinkedMesh::extrude_vertex( const EdgeHandle edge, vec3 offset )
{
    assert( edge->next );
    assert( edge->opposing == nullptr );
    /*
        1          1
        |          |\
        |          | \
        |          |  \
        0   X      0---X
            ^          ^
            | new      | new

    */

    VertexHandle vertex[] =
    {
        edge->vertex,
        edge->next->vertex
    };

    std::vector<EdgeHandle> edge_loop;
    edge_loop.emplace_back( add_halfedge( vertex[1] ) );
    link_edges( edge_loop.back(), edge );
    edge_loop.emplace_back( add_halfedge( vertex[0] ) );
    edge_loop.emplace_back( add_halfedge( add_vertex( vertex[0]->position + offset ) ) );

    return add_face( edge_loop );
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::FaceHandle LinkedMesh::extrude_edge( const EdgeHandle edge, const vec3 & offset )
{
    assert( edge->next );
    assert( edge->opposing == nullptr );

    /*
      1   X      1---X
      |          |   |
      |          |   |
      |          |   |
      0   X      0---X <- new
          ^
          | new

    */

    VertexHandle vertex[] =
    {
        edge->vertex,
        edge->next->vertex
    };

    std::vector<EdgeHandle> edge_loop;
    edge_loop.emplace_back( add_halfedge( vertex[1] ) );
    link_edges( edge_loop.back(), edge );
    edge_loop.emplace_back( add_halfedge( vertex[0] ) );
    edge_loop.emplace_back( add_halfedge( add_vertex( vertex[0]->position + offset, vertex[0]->color ) ) );
    edge_loop.emplace_back( add_halfedge( add_vertex( vertex[1]->position + offset, vertex[1]->color ) ) );

    return add_face( edge_loop );
}

/// ////////////////////////////////////////////////////////////////////////////
LinkedMesh::FaceHandle LinkedMesh::extrude_face( const FaceHandle face, const vec3 & offset )
{
    std::vector<FaceHandle> extruded_faces;
    std::vector<EdgeHandle> cap_edges;
    const auto first_edge = face->edge;
    auto edge = face->edge;

    do
    {
        extruded_faces.emplace_back( extrude_edge( edge, offset ) );
    }
    while( ( edge = edge->next ) != first_edge );

    for( unsigned int i = 0; i < extruded_faces.size(); ++i )
    {
        /// the first edge should be linked to the base
        // assert( extruded_faces[i]->edge->opposing->opposing == extruded_faces[i]->edge );

        auto next_in_loop = ( i+1 )%extruded_faces.size();

        //std::cout << " [0]:" << extruded_faces[i]->edge << " [1]:" << extruded_faces[i]->edge->next << " [2]:" << extruded_faces[i]->edge->next->next << " [3]:" << extruded_faces[i]->edge->next->next->next << std::endl;
        //std::cout << " [ ]:" << extruded_faces[i]->edge->opposing << " [ ]:" << extruded_faces[i]->edge->next->opposing << " [ ]:" << extruded_faces[i]->edge->next->next->opposing << " [ ]:" << extruded_faces[i]->edge->next->next->next->opposing << std::endl;

        link_edges( extruded_faces[i]->edge->next->next->next, extruded_faces[next_in_loop]->edge->next );
        //assert( extruded_faces[i]->edge->next->vertex->position == extruded_faces[next_in_loop]->edge->next->next->opposing->vertex->position );

        cap_edges.emplace_back( add_halfedge( extruded_faces[i]->edge->next->next->vertex ) );
        link_edges( extruded_faces[i]->edge->next->next, cap_edges.back() );
    }

    return add_face( cap_edges );
}

/// ////////////////////////////////////////////////////////////////////////////
void LinkedMesh::points( std::vector<float> & position_buffer, std::vector<float> & color_buffer )
{
    for( const auto & vertex : m_vertices )
    {
        position_buffer.emplace_back( vertex->position[0] );
        position_buffer.emplace_back( vertex->position[1] );
        position_buffer.emplace_back( vertex->position[2] );
        position_buffer.emplace_back( 1.0f );

        color_buffer.emplace_back( vertex->color[0] );
        color_buffer.emplace_back( vertex->color[1] );
        color_buffer.emplace_back( vertex->color[2] );
        color_buffer.emplace_back( 1.0f );
    }
}

/// ////////////////////////////////////////////////////////////////////////////
void LinkedMesh::triangles( std::vector<Mesh::Vertex> & vertices )
{
    auto barycenter = vec3();

    for( const auto & face : m_faces )
    {
        if( face->edge == nullptr ) continue;

        //compute_normal( face.get() );

        const auto first_edge = face->edge;
        auto edge = first_edge;
        unsigned int vertex_count = 0;
        unsigned int barycenter_index = 0;

        do
        {
            barycenter = vec3( 0.0f, 0.0f, 0.0f );
            barycenter[barycenter_index++] = 1.0f;

            vertices.push_back( Mesh::Vertex( vec4( vec3( edge->vertex->position ), 1.0f ), edge->vertex->color, face->normal, edge->texcoord, barycenter, edge->vertex->light ) );
            if( ++vertex_count % 3 == 0 )
            {
                barycenter_index = 0;
                barycenter = vec3( 0.0f, 0.0f, 0.0f );
                barycenter[barycenter_index++] = 1.0f;
                vertices.push_back( Mesh::Vertex( vec4( vec3( edge->vertex->position ), 1.0f ), edge->vertex->color, face->normal, edge->texcoord, barycenter, edge->vertex->light ) );
            }
            edge = edge->next;
        }
        while( edge != first_edge );
        vertices.push_back( Mesh::Vertex( vec4( vec3( edge->vertex->position ), 1.0f ), edge->vertex->color, face->normal, edge->texcoord, vec3( 0.0f, 0.0f, 1.0f ), edge->vertex->light ) );
    }
}

/// ////////////////////////////////////////////////////////////////////////////
void LinkedMesh::triangles( std::vector<float> & position_buffer, std::vector<float> & color_buffer )
{
    /// every face stored
    for( const auto & face : m_faces )
    {
        if( face->edge == nullptr )
        {
            continue;
        }

        /// starting edge of this edgeloop, used as break condition
        const auto first_edge = face->edge;

        /// current edge in the process
        auto edge = first_edge->next;

        ///count the number of vertices triangulated
        unsigned int vertex_counter = 0;

        /// every edge in edgeloop
        while( edge != first_edge )
        {
            assert( edge->vertex );

            position_buffer.emplace_back( edge->vertex->position[0] );
            position_buffer.emplace_back( edge->vertex->position[1] );
            position_buffer.emplace_back( edge->vertex->position[2] );
            position_buffer.emplace_back( 1.0f );

            color_buffer.emplace_back( edge->vertex->color[0] );
            color_buffer.emplace_back( edge->vertex->color[1] );
            color_buffer.emplace_back( edge->vertex->color[2] );
            color_buffer.emplace_back( edge->vertex->color[3] );

            if( ( ++vertex_counter % 2 ) == 0 )
            {
                position_buffer.emplace_back( first_edge->vertex->position[0] );
                position_buffer.emplace_back( first_edge->vertex->position[1] );
                position_buffer.emplace_back( first_edge->vertex->position[2] );
                position_buffer.emplace_back( 1.0f );

                color_buffer.emplace_back( first_edge->vertex->color[0] );
                color_buffer.emplace_back( first_edge->vertex->color[1] );
                color_buffer.emplace_back( first_edge->vertex->color[2] );
                color_buffer.emplace_back( first_edge->vertex->color[3] );

                if( edge->next == first_edge )
                {
                    break;
                }
            }
            else
            {
                edge = edge->next;
            }
        }
    }
}

/// ////////////////////////////////////////////////////////////////////////////
void LinkedMesh::clear()
{
    m_vertices.clear();
    m_edges.clear();
    m_faces.clear();
}

/// ////////////////////////////////////////////////////////////////////////////
void LinkedMesh::reset()
{
    auto reset_face = []( std::unique_ptr<Face> & face )
    {
        face->edge = nullptr;
        face->initialized = false;

        return face->id;
    };

    auto reset_edge = []( std::unique_ptr<Edge> & edge )
    {
        edge->face = nullptr;
        edge->next = nullptr;
        edge->opposing = nullptr;
        edge->vertex = nullptr;
        edge->initialized = false;

        return edge->id;
    };

    auto reset_vertex = []( std::unique_ptr<Vertex> & vertex )
    {
        vertex->color = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
        vertex->position = vec3( 0.0f, 0.0f, 0.0f );
        vertex->initialized = false;

        return vertex->id;
    };

//			for( auto & vertex : m_vertices )
//			{
//				if( vertex->initialized )
//				{
//					reset_vertex( vertex );
//					m_free_vertices.push_back( vertex->id );
//				}
//			}

    for( auto & edge : m_edges )
    {
        if( edge->initialized )
        {
            reset_edge( edge );
            m_free_edges.push_back( edge->id );
        }
    }

    for( auto & face : m_faces )
    {
        if( face->initialized )
        {
            reset_face( face );
            m_free_faces.push_back( face->id );
        }
    }
}

/// /////////////////////////////////////////////////////////////////////////
void LinkedMesh::compute_normal( const FaceHandle face )
{
    assert( face != nullptr );

    std::vector<VertexHandle> v;

    const auto first_edge = face->edge;
    auto edge = first_edge;

    do
    {
        v.push_back( edge->vertex );
        edge = edge->next;
    }
    while( edge != first_edge );

    auto vector1 = v[0]->position - v[1]->position;
    auto vector2 = v[0]->position - v[2]->position;
    face->normal = glm::normalize( glm::cross( vector1, vector2 ) );
}

/// /////////////////////////////////////////////////////////////////////////
void LinkedMesh::compute_barycenter( const vec3 & p, const vec3 & a, const vec3 & b, const vec3 & c, float & u, float & v, float & w )
{
    vec3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = glm::dot( v0, v0 );
    float d01 = glm::dot( v0, v1 );
    float d11 = glm::dot( v1, v1 );
    float d20 = glm::dot( v2, v0 );
    float d21 = glm::dot( v2, v1 );
    float denom = d00 * d11 - d01 * d01;
    v = ( d11 * d20 - d01 * d21 ) / denom;
    w = ( d00 * d21 - d01 * d20 ) / denom;
    u = 1.0f - v - w;
}
