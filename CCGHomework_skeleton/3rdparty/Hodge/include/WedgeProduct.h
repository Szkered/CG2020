/*!
 *  \file BaseHolomorphicForm.h
 *  \brief Algorithm for computing holomorphic 1-forms, Hodge star operator
 *  \author David Gu
 *  \date Document 10/12/2010
 *
 *  Algorithm for computing holomorphic 1-forms, Hodge Star operator
 *
 */
/********************************************************************************************************************************
 *
 *      BaseHolomorphic 1-form Class
 *
 *       Copyright (c) Stony Brook University
 *
 *    Purpose:
 *
 *       Compute the holomorphic 1-forms
 *
 *       David Gu June 27, 2008,  gu@cs.stonybrook.edu
 *
 *
 *    Input:
 *
 *           Original mesh, the mesh cut through a shortest path connecting one inner boundary and the exterior boundary
 *
 *    Output:
 *
 *           Closed non-exact Harmonic 1-form. and the mesh with UV coordinates.
 *
 *********************************************************************************************************************************/

#ifndef _BASE_HOLOMORPHIC_FORM_H_
#define _BASE_HOLOMORPHIC_FORM_H_

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <list>
#include <math.h>
#include <queue>
#include <vector>

#include "HodgeDecompositionMesh.h"

#ifdef _WIN32
#define EXPORTIT __declspec(dllexport)
#else
#define EXPORTIT
#endif

namespace MeshLib
{

/*! \brief CWedgeOperator class
 *
 *	Wedge Star Operator
 */
class EXPORTIT CWedgeOperator
{
  public:
    using M = CHodgeDecompositionMesh;

    /*! CWedgeOperator constructor
     * \param solid0 harmonic 1-form \f$\omega_0\f$
     * \param solid1 harmonic 1-form \f$\omega_1\f$
     */
    CWedgeOperator(M *mesh0, M *mesh1);
    /*! CWedgeOperator destructor
     */
    ~CWedgeOperator();
    /*! wedge product
     * \return
     *	\f[
                            \int \omega_0 \wedge \omega_1
    \f]
     */
    double wedge_product();
    /*! wedge product
     * \return
     *	\f[
                            \int \omega_0 \wedge {}^*\omega_1
    \f]
     */
    double wedge_star_product();

  private:
    /*! Two input harmonic 1-forms $\f\omega_0,\omega_1\f$. */
    M *m_pMesh[2];
};

/*! \brief CHolomorphicForm class
 *
 *	Compute holomorphic forms on a mesh
 */
class EXPORTIT CBaseHolomorphicForm
{
  public:
    using M = CHodgeDecompositionMesh;

    /*! CHolomorphicForm constructor
     *	\param meshes the list of meshes with stores the harmonic 1-forms, which form the basis
     *   of the first cohomology group of the mesh
     */
    CBaseHolomorphicForm(std::vector<M *> &meshes);
    /*! CHolomorphicForm destructor
     */
    ~CBaseHolomorphicForm();
    /*!	The list of meshes storing harmonic form bases
     */
    std::vector<M *> &meshes()
    {
        return m_meshes;
    };

    /*! Compute the conjugate harmonic 1-forms for the base harmonic 1-forms by solving the equation,
            Assume \f$ {}^*\omega_i = \sum_j \lambda_{ij} \omega_j \f$, then
            \f[
                    \int_M \omega_k \wedge {}^*\omega_i = \sum_j \lambda_{ij} \int_M \omega_k \wedge \omega_j
            \f]
            the left hand side can be computed using face-vector represenation.
    */
    void conjugate();

    void conjugate(std::vector<M *> &h_meshes);

  protected:
    /*!	The list of meshes storing harmonic form bases
     */
    std::vector<M *> m_meshes;
};

} // namespace MeshLib
#endif