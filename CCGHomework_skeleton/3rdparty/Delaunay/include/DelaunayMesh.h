/*! \file DelaunayMesh.h
 *   \brief Mesh Generator using Delaunay Triangulation
 *   \date  Documented on 10/14/2010
 *   \author David Gu
 *   Generate planar triangular mesh with input PLSG (Piecewise Linear Segment Graph )
 */

#ifndef _DELAUNAY_MESH_H_
#define _DELAUNAY_MESH_H_

#include <map>
#include <queue>
#include <vector>

#include "DelaunayHalfEdge.h"
#include "Mesh/BaseMesh.h"
#include "Mesh/DynamicMesh.h"
#include "Mesh/Boundary.h"
#include "Mesh/Iterators.h"
#include "Poly.h"

#ifdef _WIN32
#define EXPORTIT __declspec(dllexport)
#else
#define EXPORTIT
#endif

#define VERYLARGE 1e+10

#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef TWOPI
#define TWOPI (2.0 * PI)
#endif

namespace MeshLib
{

/*! \brief CDeluanayMesh class
 *
 *   Delaunay triangulation Algorithm and Mesh structure
 */
class EXPORTIT CDelaunayMesh : public CDynamicMesh<CDelaunayVertex, CDelaunayEdge, CDelaunayFace, CDelaunayHalfEdge>
{
  public:
    typedef CDelaunayVertex CVertex;
    typedef CDelaunayEdge CEdge;
    typedef CDelaunayFace CFace;
    typedef CDelaunayHalfEdge CHalfEdge;

    // shorten
    typedef CDelaunayVertex V;
    typedef CDelaunayEdge E;
    typedef CDelaunayFace F;
    typedef CDelaunayHalfEdge H;

    typedef CBoundary<V, E, F, H> CBoundary;
    typedef CLoop<V, E, F, H> CLoop;
    typedef MeshVertexIterator<V, E, F, H> MeshVertexIterator;
    typedef MeshFaceIterator<V, E, F, H> MeshFaceIterator;
    typedef MeshEdgeIterator<V, E, F, H> MeshEdgeIterator;

    typedef VertexVertexIterator<V, E, F, H> VertexVertexIterator;
    typedef VertexEdgeIterator<V, E, F, H> VertexEdgeIterator;
    typedef VertexFaceIterator<V, E, F, H> VertexFaceIterator;
    typedef VertexOutHalfedgeIterator<V, E, F, H> VertexOutHalfedgeIterator;

    typedef FaceVertexIterator<V, E, F, H> FaceVertexIterator;
    typedef FaceEdgeIterator<V, E, F, H> FaceEdgeIterator;
    typedef FaceHalfedgeIterator<V, E, F, H> FaceHalfedgeIterator;

  public:
    /*! CDeluanayMesh constructor */
    CDelaunayMesh();
    /*! CDeluanayMesh destructor  */
    ~CDelaunayMesh();

    /*!
     * process poly file
     * \param input input mesh file name
     * \param output output mesh file name
     */
    void ProcessPoly(const char *input, const char *output);
    /*!
     * process poly file
     * \param input CPoly
     * \param output mesh
     */
    void ProcessPoly(CPoly &poly, CDelaunayMesh &omesh);

    /*!
     * Delaunay triangulate the input points
     * \param input input mesh file name
     * \param output output mesh file name
     */
    void DelaunayTriangulate(const char *input, const char *output);

    /*!
     * Delaunay triangulate the input points
     * \param input input points
     * \param output output mesh file name
     */
    void DelaunayTriangulate(std::vector<CPoint2> &points, CDelaunayMesh &omesh);

    void GenerateMesh(std::vector<CPoint2> &points, CDelaunayMesh &omesh);

  protected:
    /*!	Construct initial triangle
     *  \param grading false means the area is considered
     */
    void _initialize(bool grading = false);
    /*!	Insert one point to the current Delaunay triangulation
     *  update the Delaunay Triangulation
     *  \param pt the newly insterted point on the plane
     *  \param pV the newly generated vertex
     */
    virtual bool _insert_vertex(CPoint2 &pt, V *&pV);
    /*!	Check if any segment is missing in the current Delaunay triangulation
     *   if so, split the segment to 2 halves, and insert the midpoint to the current DT (Delaunay Triangulation)
     *   update the DT.
     */
    void _insert_missing_segments();
    /*! Find the triangle with the worst quality
     *	compute its Steiner point
     *  if the Steiner point encroaches any segment, subdivide the segment
     *  otherwise, insert the Steiner point to the current DT
     *  \param lower_angle threshould for angle bound
     */
    void RemoveBadTriangles(double lowerAngle = 20);
    /*! Find the triangle with the worst quality
     *	compute its Steiner point
     *	if the Steiner point doest encroach any segment
     *  insert the Steiner point to the current DT.
     *  if the Steiner point encroaches any segment,
     *  if the segment is not frozen then subdivide the segment, and continue.
     *  if the segment is frozen, insert the Steinter point to the current DT.
     *  \param lower_angle threshould for angle bound
     */
    void RemoveBadTrianglesWithFrozenSegments(double lowerAngle = 20);
    /*!	Classify all the faces to be inside or outside.
     *  a. all faces attaching to the initial bounding vertices are outside
     *  b. the faces adjacent to the center of holes are outside
     *  c. flood the outside label till reach the segments
     */
    void _classify_inside_outside();

  public:
    /*!  Save inside faces to another mesh
     *   \param output_mesh the output mesh
     */
    void Convert(CDelaunayMesh &output_mesh);

  protected:
    /*!	Insert a new vertex on an edge
     *  \param e the edge to be split
     *  \param pt the position of the new vertex
     */
    virtual V *_insert_vertex_on_edge(E *e, CPoint2 &pt);

    /*!	Split a segment and the edge attaching to it
     *	\param pS the segment to be split
     */
    void _split_edge_on_segment(CSegment<V> *pS);

    /*!	Test if an edge is legal, if not, swap it
     *  and test whether the new neighbors are legal
     *  \param pV the new vertex
     *  \param pE the edge to be tested
     *  \param pF the face bound by pV and pE
     *  \param level recursion level
     */
    bool _legalize_edge(const V *pV, E *pE, F *f, int level);

  protected:
    /*! Locate a point in DT, find the face containing it
     *	\param pt the point to be located
     *   \param q
     */
    F *_locate_point(CPoint2 &pt, CPoint &q);
    /*! check if the circle through v0, v1 and centered at their midpoint contains v
     *  \param v0, v1 the end vertices of an edge
     *  \param v the point to be tested
     */
    bool _encroach(V *v0, V *v1, CPoint2 v);
    /*! Test if the circum circle of e includes the vertices of adjacent faces against e
     *  \param e edge to be tested
     */
    bool _encroach(E *e);

    /*! Find an edge with end vertices v0, v1
     * \param v0, v1 end vertices
     */
    E *_find_edge(V *v0, V *v1);
    /*! Find a segment with end vertices v0, v1
     *  \param v0, v1 end vertices
     */
    CSegment<V> *_find_segment(V *v0, V *v1);
    /*! create a new segment with end vertices v0, v1
     *  \param v0, v1 end vertices
     */
    CSegment<V> *_create_segment(V *v0, V *v1);

    /*!	Compute the statistics of triangle of face
     *   \param face input triangle
     */
    void _statistics(F *face);
    /*!	Compute the statistics of triangle of [v0,v1,v2]
     *   \param v0,v1,v2 input triangle
     *   \param minAngle, quality output minimal angle and quality of the triangle
     */
    virtual void _statistics(V *v0, V *v1, V *v2, double &minAngle, double &quality);
    /*!
     *	freeze edges on segments, they can not be flipped any more
     */
    void _freeze_edges_on_segments();

  protected:
    /*! oriented area bounded by face [a,b,c]
     * \param a,b,c points of a triangle
     */
    double __orient2d(CPoint2 &a, CPoint2 &b, CPoint2 &c);
    /*! circumcenter of face
     *	\param face input triangle
     */
    CPoint2 __circumcenter(F *face);
    /*!	angle difference from -PI to PI
     *	\param a1, a0 rotate a0 to a1
     */
    double __angle_difference(double a1, double a0);
    /*! wheither the triangle [v0,v1,v2] contains bounding vertices
     *  \param v0,v1,v2 triangle vertices
     */
    bool __contains_bounding_vertices(V *v0, V *v1, V *v2);
    /*!	whether the corner angle at v1 in face [v0,v1,v2] is bounded by segments
     *  \param v0,v1,v2 three vertices of a triangle
     */
    bool __is_segment_bounded_angle(V *v0, V *v1, V *v2);

  protected:
    /*! maximal number of vertices to be allowed in the meshing result */
    size_t m_max_number_vertices;
    /*! input vertices of PLSG */
    int m_inputVertexNum;
    /*! the angle Lower bound */
    double m_angleLowerbound;
    /*! the quliaty lower bound */
    double m_qualityLowerbound;
    /*! starting face in point location procedure*/
    F *m_starting_face;
    /*! grading is false, consider the area; grading is true, consider the angle only*/
    bool m_grading;
    /*! vector of segments */
    std::vector<CSegment<V> *> m_segments;
    /*! vector of centers of holes */
    std::vector<CPoint2> m_hole_centers;
};

typedef CDelaunayMesh CDTMesh;

} // namespace MeshLib
#endif _DELAUNAY_MESH_H_