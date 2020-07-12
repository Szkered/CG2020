#ifndef _CUT_GRAPH_MESH_
#define _CUT_GRAPH_MESH_

#include "Mesh/BaseMesh.h"
#include "Mesh/Edge.h"
#include "Mesh/Face.h"
#include "Mesh/HalfEdge.h"
#include "Mesh/Vertex.h"

#include "Mesh/boundary.h"
#include "Mesh/iterators.h"
#include "Parser/parser.h"

namespace MeshLib
{
class CCutGraphVertex;
class CCutGraphEdge;
class CCutGraphFace;
class CCutGraphHalfEdge;

/*! \brief CCutGraphVertex class
 *
 *   Vertex class for cut graph algoritm
 *   Trait : Vertex valence
 */
class CCutGraphVertex : public CVertex
{
  public:
    /*! Constructor */
    CCutGraphVertex() : m_valence(0){};

    /*! Vertex valence */
    int &valence()
    {
        return m_valence;
    };

  protected:
    /*! Vertex valence */
    int m_valence;
};

/*! \brief CCutGraphEdge class
 *
 *   Edge class for cut graph algorithm
 *   Trait : Edge sharp
 */
class CCutGraphEdge : public CEdge
{
  public:
    /*! Constructor */
    CCutGraphEdge() : m_sharp(false){};

    /*! Sharp edge */
    bool &sharp()
    {
        return m_sharp;
    };

  protected:
    /*! Sharp edge */
    bool m_sharp;
};

/*! \brief CCutGraphFace class
 *
 *   Face class for cut graph algorithm
 *   Trait : Face touched flag
 */
class CCutGraphFace : public CFace
{
  public:
    /*! Constructor */
    CCutGraphFace() : m_touched(false){};

    /*! face touched flag */
    bool &touched()
    {
        return m_touched;
    };

    /*! face normal */
    CPoint &normal()
    {
        return m_normal;
    };

  protected:
    /*! face touched flag */
    bool m_touched;

    /*! face normal */
    CPoint m_normal;
};

/*! \brief CCutGraphHalfEdge class
 *
 *   HalfEdge class for cut graph algorithm
 */
class CCutGraphHalfEdge : public CHalfEdge
{
};

/*! \brief CCutGraphMesh class
 *
 *	Mesh class for cut graph algorithm
 *
 */
template <typename V, typename E, typename F, typename H> class TCutGraphMesh : public CBaseMesh<V, E, F, H>
{
  public:
    typedef V CVertex;
    typedef E CEdge;
    typedef F CFace;
    typedef H CHalfEdge;

    typedef CBoundary<V, E, F, H> CBoundary_;
    typedef CLoop<V, E, F, H> CLoop_;

    typedef MeshVertexIterator<V, E, F, H> MeshVertexIterator_;
    typedef MeshEdgeIterator<V, E, F, H> MeshEdgeIterator_;
    typedef MeshFaceIterator<V, E, F, H> MeshFaceIterator_;
    typedef MeshHalfEdgeIterator<V, E, F, H> MeshHalfEdgeIterator_;

    typedef VertexVertexIterator<V, E, F, H> VertexVertexIterator_;
    typedef VertexEdgeIterator<V, E, F, H> VertexEdgeIterator_;
    typedef VertexFaceIterator<V, E, F, H> VertexFaceIterator_;
    typedef VertexInHalfedgeIterator<V, E, F, H> VertexInHalfedgeIterator_;
    typedef VertexOutHalfedgeIterator<V, E, F, H> VertexOutHalfedgeIterator_;

    typedef FaceVertexIterator<V, E, F, H> FaceVertexIterator_;
    typedef FaceEdgeIterator<V, E, F, H> FaceEdgeIterator_;
    typedef FaceHalfedgeIterator<V, E, F, H> FaceHalfedgeIterator_;
};

typedef TCutGraphMesh<CCutGraphVertex, CCutGraphEdge, CCutGraphFace, CCutGraphHalfEdge> CCutGraphMesh;

} // namespace MeshLib

#endif // !_CUT_GRAPH_MESH_
