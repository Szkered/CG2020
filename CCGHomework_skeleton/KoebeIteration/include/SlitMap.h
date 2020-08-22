/*! \file SlitMap.h
 *  \brief Algorithm for computing slit mapping
 *  \author David Gu
 *  \date Documented on 10/12/2010
 *
 *	Compute slit maps
 *
 *	The inputs are holomorphic basis for the mesh \f$\{\omega_1,\omega_2,\cdots, \omega_{n}\}\f$.
 *	Suppose the boundary components are \f$\{\gamma_0,\gamma_1,\gamma_2,\cdots, \gamma_n\}\f$.
 *	Find a new holomorphic 1-form \f$\omega = \sum_k \lambda_k \omega_k\f$, such that
 *  \f[
 *		Img \int_{\gamma_0} \omega = 2\pi, 		Img \int_{\gamma_k} \omega = -2\pi,	Img \int_{\gamma_i} \omega = 0.
 *
 *  \f]
 */
/********************************************************************************************************************************
 *
 *      Harmonic Closed Form Class
 *
 *       Copyright (c) Stony Brook University
 *
 *    Purpose:
 *
 *       Compute the holomorphic 1-form for a slit map
 *
 *       David Gu June 27, 2008,  gu@cs.stonybrook.edu
 *
 *
 *      Input:
 *
 *           basis of holomorphic 1-forms, boundary loop ids, inner and outer boundaries
 *
 *      Output:
 *
 *           holomorphic 1-form, imaginary part of the integration are (+ - 2PI )
 *
 *********************************************************************************************************************************/

/*---------------------------------------------------------------------------------------------------------------------------------
#include "SlitMap/SlitMap.h"
#include "mesh/mesh.h"
#include <math.h>

using namespace MeshLib;

int main( int argc, char * argv[] )
{
   if( strcmp( argv[1], "-slit_map" ) == 0 )
  {

        std::vector<CSMMesh*> meshes;

        for( int i = 2; i < argc-3; i ++ )
        {
                CSMMesh * pMesh = new CSMMesh;
                assert( pMesh );
                pMesh->read_m( argv[i] );
                meshes.push_back( pMesh );
        }

        CSlitMap map( meshes );

        int c1 = atoi( argv[argc-3] );
        int c2 = atoi( argv[argc-2] );

        map._slit_map( c1,c2 );

        meshes[0]->write_m( argv[argc-1] );

        for( size_t i = 0; i < meshes.size(); i ++ )
        {
                delete meshes[i];
        }
        return 0;
  }
}
----------------------------------------------------------------------------------------------------------------------------------*/

#ifndef _SLIT_MAP_H_
#define _SLIT_MAP_H_

//#include "SlitMapMesh.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace MeshLib
{

/*! \brief CSlitMap class
 *
 *	Algorithm for computing slit maps
 */
template <typename M> class CSlitMap
{
  public:
    /*! CSlitMap constructor
     *
     * \param meshes the basis of holomorphic 1-forms
     */
    CSlitMap(std::vector<M *> &meshes);
    /*! CSlitMap destructor */
    ~CSlitMap();
    /*! Compute the slit map, such that loop c1 becomes the exterior circle, loop c2 becomes the interior circle
     * \param c1 loop id , which becomes the exterior circle
     * \param c2 loop id , which becomes the interior circle
     * other boundary loops become concentric circular slits
     */
    void _slit_map(int c1, int c2);

  protected:
    /*! holomorphic 1-form basis */
    std::vector<M *> &m_meshes;
    /*! base mesh */
    M *m_pMesh;
    /*! integrate a holomorphic 1-form along a boundary loop pL
     * \param pMesh holomorphic 1-form
     * \param pL    the boundary loop
     * \return the integration of the holomorphic 1-form along the boundary loop
     */
    CPoint2 _integrate(M *pMesh, typename M::CLoop *pL);
    /*! the boundary of the input mesh */
    typename M::CBoundary m_boundary;
};

// CSlitMap constructor
//\param meshes the input holomorphic 1-form basis

template <typename M>
CSlitMap<M>::CSlitMap(std::vector<M *> &meshes) : m_meshes(meshes), m_pMesh(meshes[0]), m_boundary(m_pMesh)
{
    assert(m_meshes.size() > 0);
}

// CSlitMap destructor
template <typename M> CSlitMap<M>::~CSlitMap()
{
}

// Integrate a holomorphic 1-form pMesh along a boundary loop pL
template <typename M> CPoint2 CSlitMap<M>::_integrate(M *pMesh, typename M::CLoop *pL)
{
    CPoint2 sum(0, 0);

    for (typename std::list<typename M::CHalfEdge *>::iterator hiter = pL->halfedges().begin();
         hiter != pL->halfedges().end(); hiter++)
    {
        typename M::CHalfEdge *he = *hiter;
        typename M::CVertex *v1 = pMesh->halfedgeSource(he);
        typename M::CVertex *v2 = pMesh->halfedgeTarget(he);

        int id1 = v1->id();
        int id2 = v2->id();

        typename M::CVertex *w1 = pMesh->idVertex(id1);
        typename M::CVertex *w2 = pMesh->idVertex(id2);

        typename M::CEdge *e = pMesh->vertexEdge(w1, w2);

        if (pMesh->edgeVertex1(e) == w1)
        {
            sum = sum + e->duv();
        }
        else
        {
            sum = sum - e->duv();
        }
    }

    return sum;
}

/*! Compute the slit map, by finding a holomorphic 1-form, the integration along c1 loop is +1, the integration
 *  along c2 is -1
 *
 *  All the holomorphic 1-forms are stored in m_meshes, \omega_j corresponds m_meshes[j], edge->duv()
 *  the result holomorhic 1-form is stored in m_pMesh
 */
template <typename M> void CSlitMap<M>::_slit_map(int c1, int c2)
{

    typename std::vector<typename M::CLoop *> &loops = m_boundary.loops();

    int num = m_meshes.size();

    if (c1 < 0 || c2 < 0 || (size_t)c1 > loops.size() || (size_t)c2 > loops.size())
    {
        printf("The loop Id should between [0..%d]\n", loops.size());
        return;
    }

    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(num, num);
    Eigen::VectorXd b(num);
    b.setZero();
    b(c1) = 1;
    if (num > 1)
    {
        b(c2) = -1;
    }

    // insert your code
    // set matrix A, A(i,j) = imaginary part of the integration of \omega_j along loops[i];

    for (int i = 0; i < num; i++)
    {
        typename M::CLoop *pL = loops[i];
        // insert your code here
        for (int j = 0; j < num; j++)
        {
            M *pM = m_meshes[j];
        }
    }

    // solve the linear combination coefficients
    Eigen::VectorXd x = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);

    // insert your code here
    // linear combine \omega[j] * x(j) and store the result in m_pMesh

    for (typename M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); ++eiter)
    {
        typename M::CEdge *e = *eiter;
        typename M::CVertex *v1 = m_pMesh->edgeVertex1(e);
        typename M::CVertex *v2 = m_pMesh->edgeVertex2(e);
        CPoint2 duv(0, 0);
        for (size_t k = 0; k < m_meshes.size(); k++)
        {
            M *pM = m_meshes[k];
        }
        e->duv() = duv;
    }
}

} // namespace MeshLib
#endif _SLIT_MAP_H_