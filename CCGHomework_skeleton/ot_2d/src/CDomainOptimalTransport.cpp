/*!  \file   DomainOptimalTransport.cpp
 *   \brief  OptimalTransport
 *   \author David Gu
 *   \date   documented on 05/18/2020
 *
 *   Class for Planar Optimal Transportation, Semi-Discrete Algorithm
 */

#include <Eigen/Eigen>

#include "CDomainOptimalTransport.h"
#include "Geometry/Point.h"
#include "Mesh/Edge.h"
#include "Mesh/Face.h"
#include "Mesh/HalfEdge.h"
#include "Mesh/Vertex.h"
#include "OTMesh.h"

namespace MeshLib
{
CDomainOptimalTransport::CDomainOptimalTransport(COMTMesh *pMesh) : CBaseOT(pMesh)
{
    m_pWDT = NULL;
    m_domainTr = NULL;
    m_outputTr = NULL;
}

CDomainOptimalTransport ::~CDomainOptimalTransport()
{
    delete m_domainTr;
    delete m_outputTr;
}

// initialize the potential function as quadratic
void CDomainOptimalTransport::set_target_measure_to_uniform()

{
    std::cout << "setting target measure to 1/n" << std::endl;
    _set_target_measure(m_pMesh, total_target_area, true);
}

void CDomainOptimalTransport::find_singularities(COMTMesh *pInput)
{
    // Mark all sharp flags false
    for (COMTMesh::MeshEdgeIterator eiter(pInput); !eiter.end(); ++eiter)
    {
        COMTEdge *pE = *eiter;
        pE->sharp() = false;
    }

    double threshold = 0.01;
    std::cout << "finding singualrities with threshold " << threshold << std::endl;
    for (COMTMesh::MeshEdgeIterator eiter(pInput); !eiter.end(); ++eiter)
    {
        COMTEdge *pE = *eiter;

        // CVertex v1 = pInput->edgeVertex1(pE);
        // CVertex v2 = pInput->edgeVertex1(pE);

        CPoint n1 = pInput->edgeVertex1(pE)->normal();
        CPoint n2 = pInput->edgeVertex2(pE)->normal();
        double cos_theta = (n1 * n2) / (n1.norm() * n2.norm());
        if (1 - cos_theta > threshold)
            pE->sharp() = true;
    }
}

void CDomainOptimalTransport::_initialize(bool uniform)
{

    /*! compute the domain triangulation, a convex polygon */
    __detri2_generate_disk(m_domainTr, total_target_area);

    /*! set the target measure */
    _set_target_measure(m_pMesh, total_target_area, uniform);

    /*! initialize the vertex index, starting from zero */
    index(m_pMesh);

    for (COMTMesh::MeshVertexIterator viter(m_pMesh); !viter.end(); viter++)
    {
        COMTMesh::CVertex *pv = *viter;
        ids[pv->index()] = pv->id();
    }

    /* initialize the vertex weight */
    for (COMTMesh::MeshVertexIterator viter(m_pMesh); !viter.end(); viter++)
    {
        COMTMesh::CVertex *pv = *viter;
        pv->weight() = 0;
    }

    /*! normalize the vertex uv coordinates */
    _normalize_uv(m_pMesh);

    /*! Compute Weighted Delaunay of Base Mesh Vertices */
    __detri2_WDT(m_pMesh, &m_outputTr);

    /*! convert the Weighted Delaunay Triangulation to a mesh */
    __detri2_to_mesh(m_outputTr, m_domainTr, m_pWDT);

    /*! copy vertex_weight, vertex_target_area, vertex_index from base mesh to the Weighted Delaunay Triangulation Mesh
     */
    _copy_mesh(m_pMesh, m_pWDT);
};

/*! Gradient Descende method to compute the OT Map */
void CDomainOptimalTransport::__gradient_descend(
    COMTMesh *pInput,  // input  mesh, the vertex->target_area, dual_area need to be set
    COMTMesh *&pOutput // output mesh, the vertex->target_area, dual_area, dual_cell, uv (dual cell center) have been
                       // set, the connectivity is updated
)
{

    // #pragma omp parallel for
    //     for (int i = 0; i < V.size(); ++i)
    //     {
    //         COMTMesh::CVertex *pv = V.at(i);
    //         // for (int i = 0; i < m_pMesh->numVertices(); ++i)
    //         // {
    //         //     int id = ids[i];
    //         //     COMTMesh::CVertex *pv = m_pMesh->idVertex(id);
    //         double grad = -(pv->target_area() - pv->dual_area());
    //         pv->update_direction() = grad;
    //     }

    /*! compute the gradient, which equals to (target_area - dual_area) */
    for (COMTMesh::MeshVertexIterator viter(pInput); !viter.end(); viter++)
    {
        COMTMesh::CVertex *pv = *viter;
        // to compute the gradient
        double grad = -(pv->target_area() - pv->dual_area());
        pv->update_direction() = grad;
    }

    /*! set initial step length */
    double step_length = 0.1;
    /*! pointer to the output WDT */
    detri2::Triangulation *pTr = NULL;
    /*! temparary mesh */
    COMTMesh *pMesh = NULL;

    /*! damping iteration */
    while (true)
    {

        // #pragma omp parallel for
        //         for (int i = 0; i < V.size(); ++i)
        //         {
        //             COMTMesh::CVertex *pv = V.at(i);
        //             // for (int i = 0; i < m_pMesh->numVertices(); ++i)
        //             // {
        //             //     int id = ids[i];
        //             //     COMTMesh::CVertex *pv = m_pMesh->idVertex(id);
        //             pv->weight() -= step_length * pv->update_direction();
        //         }

        // update the vertex weight
        for (COMTMesh::MeshVertexIterator viter(pInput); !viter.end(); viter++)
        {
            COMTMesh::CVertex *pv = *viter;
            pv->weight() -= step_length * pv->update_direction();
        }

        // compute Weighted Delaunay Triangulation
        if (!__detri2_WDT(pInput, &pTr))
        {

            // #pragma omp parallel for
            //             for (int i = 0; i < V.size(); ++i)
            //             {
            //                 COMTMesh::CVertex *pv = V.at(i);
            //                 // for (int i = 0; i < m_pMesh->numVertices(); ++i)
            //                 // {
            //                 //     int id = ids[i];
            //                 //     COMTMesh::CVertex *pv = m_pMesh->idVertex(id);
            //                 pv->weight() += step_length * pv->update_direction();
            //             }

            // if there are missing points,
            // roll back the vertex weight
            for (COMTMesh::MeshVertexIterator viter(pInput); !viter.end(); viter++)
            {
                COMTMesh::CVertex *pv = *viter;
                pv->weight() += step_length * pv->update_direction();
            }

            delete pTr;
            pTr = NULL;
            // reduce the step length by half
            step_length /= 2.0;
            // try again
            continue;
        }
        // no missing point

        // convert the detri2::Triangulation to OMTMesh
        // this will set the vertex->dual_area, vertex->dual_cell, vertex->dual_center;
        // edge->length, edge->dual_length;
        __detri2_to_mesh(pTr, m_domainTr, pMesh);
        // copy vertex->target_carea, vertex->weight, vertex->index from the input mesh
        // to the newly generated WDT mesh
        _copy_mesh(pInput, pMesh);

        // if the old WDT pointer is nonempty, delete it
        if (m_outputTr != NULL)
        {
            delete m_outputTr;
        }
        // update the WDT pointer by the current one
        m_outputTr = pTr;
        pOutput = pMesh;

        break;
    }
};

/*! Newton's method to compute the OT Map */
void CDomainOptimalTransport::__newton(
    COMTMesh *pInput, // input mesh, with vertex->target_area, vertex->dual_area, vertex->weight, vertex->index
    COMTMesh *
        &pOutput // output mesh, with copied vertex->target_area, vertex->index,
                 // and updated vertex->dual_area, vertex->weight, vertex->dual_cell, vertex->dual_center, connectivity
)
{
    /*! Use Newton's method to compute the update_direction */
    __update_direction(pInput);

    /*! set initial step length */
    // double step_length = 1.0;
    double step_length = 0.5;
    /*! pointer to the output WDT */
    detri2::Triangulation *pTr = NULL;
    /*! temparary mesh */
    COMTMesh *pMesh = NULL;

    /*! damping iteration */
    while (true)
    {

        // update the vertex weight
        for (COMTMesh::MeshVertexIterator viter(pInput); !viter.end(); viter++)
        {
            COMTMesh::CVertex *pv = *viter;
            pv->weight() -= step_length * pv->update_direction();
        }

        // compute Weighted Delaunay Triangulation
        bool success = __detri2_WDT(pInput, &pTr);

        if (!success)
        {
            // if there are missing points,
            // roll back the vertex weight
            // reduce the step length by half
            for (COMTMesh::MeshVertexIterator viter(pInput); !viter.end(); viter++)
            {
                COMTMesh::CVertex *pv = *viter;
                pv->weight() += step_length * pv->update_direction();
            }

            step_length /= 2.0;

            delete pTr;
            pTr = NULL;
            continue;
        }

        // no missing point

        // convert the detri2::Triangulation to OMTMesh
        // this will set the vertex->dual_area, vertex->dual_cell, vertex->dual_center;
        // edge->length, edge->dual_length;
        __detri2_to_mesh(pTr, m_domainTr, pMesh);
        // copy vertex->target_area, vertex->weight, vertex->index from the input mesh
        // to the newly generated WDT mesh
        _copy_mesh(pInput, pMesh);

        // if the old WDT pointer is nonempty, delete it
        if (m_outputTr != NULL)
        {
            delete m_outputTr;
        }
        // update the WDT pointer by the current one
        m_outputTr = pTr;
        pOutput = pMesh;

        break;
    }
}

} // namespace MeshLib
