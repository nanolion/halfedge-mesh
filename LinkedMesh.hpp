#pragma once

#include <vector>
#include <memory>
#include <iostream>

#include "glm/glm.hpp"

#include "vertex.hpp"
#include "edge.hpp"
#include "face.hpp"

#include "make_unique.hpp"

class LinkedMesh
{
   public:

      typedef Vertex* VertexHandle;
      typedef Edge* EdgeHandle;
      typedef Face* FaceHandle;
      typedef std::vector<EdgeHandle> EdgeLoop;

      LinkedMesh();

      ~LinkedMesh();

      /// Set the color of every vertex connected to this face
      void set_color( const FaceHandle face, const vec4 & color );

      /// set the color of a face from face index
      void set_color( const unsigned int face_index, const vec4 & color );

      /// set the texcoord of a face
      void set_texcoord( const unsigned int face_index, const std::vector<vec2> & new_texcoords );

      /// Allocate a new vertex
      VertexHandle add_vertex( const vec3 & position, const vec4 & color = vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
      
      /// Allocate a new halfedge
      EdgeHandle add_halfedge( const VertexHandle vertex );

      /// Add a new face to the mesh from 3 points and 1 color
      FaceHandle add_face( const vec3 & p0, const vec3 & p1, const vec3 & p2, const vec4 & color = vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );

      /// Add a new face to the mesh from 4 points and 1 color
      FaceHandle add_face( const vec3 & p0, const vec3 & p1, const vec3 & p2, const vec3 & p3, const vec4 & color = vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );

      /// Add a face to the mesh from EdgeHandles of the mesh
      /// by rvalue reference example: add_face(EdgeLoop())
      FaceHandle add_face( EdgeLoop && edges );

      /// Add a face to the mesh from an edgeloop in the mesh
      /// by lvalue reference example: EdgeLoop el; add_face(el)
      FaceHandle add_face( EdgeLoop & edges );

      inline void link_edges( const EdgeHandle edge_left , const EdgeHandle edge_right )
      {
         //assert( edge_left->next );
         //assert( edge_left->next->vertex->position == edge_right->vertex->position );
         //assert( edge_left->vertex->position == edge_right->next->vertex->position );

         edge_left->opposing = edge_right;
         edge_right->opposing = edge_left;
      }

      /// bridge two edges by adding two new edges connecting them
      FaceHandle bridge_edges( const EdgeHandle edge_left, const EdgeHandle edge_right );

      FaceHandle extrude_vertex( const EdgeHandle edge, vec3 offset );

      FaceHandle extrude_edge( const EdgeHandle edge, const vec3 & offset );

      FaceHandle extrude_face( const FaceHandle face, const vec3 & offset );

      /// Fill buffers with mesh points
      void points( std::vector<float> & position_buffer, std::vector<float> & color_buffer );

      /// compute the normal for this face
      void compute_normal( const FaceHandle face );

      /// compute barycenter for triangle
      void compute_barycenter( const vec3 & p, const vec3 & a, const vec3 & b, const vec3 & c, float & u, float & v, float & w );

      /// Fill mesh vertex buffer
      void triangles( std::vector<Mesh::Vertex> & vertices );

      /// Fill buffers with mesh faces as triangles
      void triangles( std::vector<float> & position_buffer, std::vector<float> & color_buffer );

      /// clear vertices, edges and faces; deleting their allocations
      void clear();

      /// Unlink all vertices, edges and faces. Preparing the LinkedMesh for reuse
      void reset();

   private:

      /// indices of vertices which were allocated but removed from the mesh and free for reuse
      std::vector<int> m_free_vertices;

      /// indices of edges which that were allocated but removed from the mesh and free for reuse
      std::vector<int> m_free_edges;

      /// indices of faces which that were allocated but removed from the mesh and free for reuse
      std::vector<int> m_free_faces;

      /// allocated vertices
      std::vector<std::unique_ptr<Vertex>> m_vertices;

      /// allocated edges
      std::vector<std::unique_ptr<Edge>> m_edges;

      /// allocated faces
      std::vector<std::unique_ptr<Face>> m_faces;
};
