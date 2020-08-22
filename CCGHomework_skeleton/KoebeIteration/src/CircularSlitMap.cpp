#include <vector>
#include <list>
#include "HodgeDecomposition.h"
#include "HodgeDecompositionMesh.h"
#include "WedgeProduct.h"
#include "SlitMap.h"
#include "CircularSlitMap.h"

using namespace MeshLib;

/*! Normalize g_mesh
 * \param pMesh the input g_mesh
 */
void MeshLib::normalizeMesh(CHodgeDecompositionMesh *pMesh)
{
    CPoint s(0, 0, 0);
    for (CHodgeDecompositionMesh::MeshVertexIterator viter(pMesh); !viter.end(); ++viter)
    {
        CHodgeDecompositionVertex *v = *viter;
        s = s + v->point();
    }
    s = s / pMesh->numVertices();

    for (CHodgeDecompositionMesh::MeshVertexIterator viter(pMesh); !viter.end(); ++viter)
    {
        CHodgeDecompositionVertex *v = *viter;
        CPoint p = v->point();
        p = p - s;
        v->point() = p;
    }

    double d = 0;
    for (CHodgeDecompositionMesh::MeshVertexIterator viter(pMesh); !viter.end(); ++viter)
    {
        CHodgeDecompositionVertex *v = *viter;
        CPoint p = v->point();
        for (int k = 0; k < 3; k++)
        {
            d = (d > fabs(p[k])) ? d : fabs(p[k]);
        }
    }

    for (CHodgeDecompositionMesh::MeshVertexIterator viter(pMesh); !viter.end(); ++viter)
    {
        CHodgeDecompositionVertex *v = *viter;
        CPoint p = v->point();
        p = p / d;
        v->point() = p;
    }
};

/*! Compute the face normal and vertex normal
 * \param pMesh the input g_mesh
 */
void MeshLib::computeNormal(CHodgeDecompositionMesh *pMesh)
{
    // return;

    for (CHodgeDecompositionMesh::MeshVertexIterator viter(pMesh); !viter.end(); ++viter)
    {
        CHodgeDecompositionVertex *v = *viter;
        CPoint n(0, 0, 0);
        for (CHodgeDecompositionMesh::VertexFaceIterator vfiter(v); !vfiter.end(); ++vfiter)
        {
            CHodgeDecompositionFace *pF = *vfiter;

            CPoint p[3];
            CHalfEdge *he = pF->halfedge();
            for (int k = 0; k < 3; k++)
            {
                p[k] = he->target()->point();
                he = he->he_next();
            }

            CPoint fn = (p[1] - p[0]) ^ (p[2] - p[0]);
            pF->normal() = fn / fn.norm();
            n += fn;
        }

        n = n / n.norm();
        // v->normal() = n;
    }
};

/*!
 * calculate holomorphic 1-form for closed meshes
 */
void MeshLib::calc_holo_1_form_closed_mesh(const std::string &input_mesh)
{

    /* global g_mesh */
    CHodgeDecompositionMesh *g_domain_mesh = NULL;
    std::vector<CHodgeDecompositionMesh *> g_meshes; // exact holomorphic 1-forms
    std::vector<CHodgeDecompositionMesh *> h_meshes; // closed, non-exact holomorphic 1-forms
    CHodgeDecomposition g_mapper;

    using M = CHodgeDecompositionMesh;
    M *pM = new M;
    pM->read_m(input_mesh.c_str());
    g_meshes.push_back(pM);

    int Euler = pM->numVertices() + pM->numFaces() - pM->numEdges();
    int genus = (2 - Euler) / 2;

    for (int i = 1; i < 2 * genus; i++)
    {
        M *pM = new M;
        pM->read_m(input_mesh.c_str());
        g_meshes.push_back(pM);
    }

    for (std::vector<M *>::iterator iter = g_meshes.begin(); iter != g_meshes.end(); iter++)
    {
        M *pM = *iter;
        normalizeMesh(pM);
        computeNormal(pM);
        g_mapper.set_mesh(pM);
        g_mapper.random_harmonic_form();
    }

    CBaseHolomorphicForm holo(g_meshes);
    holo.conjugate();

    g_mapper.integration(g_meshes[0], g_domain_mesh);
}

/*!
 *   compute the polar map, the input mesh
 *   vertex->uv of the exterior boundary is in the interval
 *   on the imaginary axis [0,i]
 */
void MeshLib::polar_map(CHodgeDecompositionMesh *pMesh)
{
    double pi = 3.1415926535;
    using M = CHodgeDecompositionMesh;
    for (M::MeshVertexIterator viter(pMesh); !viter.end(); viter++)
    {
        M::CVertex *pv = *viter;
        // insert your code here
        CPoint2 uv;
        pv->uv() = uv;
    }
}

/*!
 *   calculate the holomorphic 1-form for a genus zero mesh
 *   with multiple boundaries
 */

void MeshLib::calc_holo_1_form_open_mesh(const std::string &input_mesh,                    // input mesh name
                                         std::vector<CHodgeDecompositionMesh *> &g_meshes, // exact harmonic 1-forms
                                         std::vector<CHodgeDecompositionMesh *> &h_meshes, // closed harmonic 1-forms
                                         std::string &output_mesh_name)
{
    using M = CHodgeDecompositionMesh;

    CHodgeDecomposition g_mapper;

    // read the first mesh
    M *pM = new M;
    pM->read_m(input_mesh.c_str());
    g_meshes.push_back(pM);

    normalizeMesh(pM);
    computeNormal(pM);

    // find the number of boundary components
    M::CBoundary bnd(pM);
    size_t n = bnd.loops().size() - 1;

    // read in more meshes, for computing exact harmonic 1-forms
    for (size_t i = 1; i < n; i++)
    {
        pM = new M;
        pM->read_m(input_mesh.c_str());
        g_meshes.push_back(pM);
    }
    // read in n meshes, for non-exact harmonic 1-forms
    for (size_t i = 0; i < n; i++)
    {
        pM = new M;
        pM->read_m(input_mesh.c_str());
        h_meshes.push_back(pM);
    }

    // compute exact harmonic 1-forms
    for (size_t i = 0; i < n; i++)
    {
        M *pM = g_meshes[i];
        g_mapper.set_mesh(pM);
        std::cout << std::endl << "Exact Harmonic 1-form " << i << std::endl << std::endl;
        g_mapper.exact_harmonic_form(i + 1);
    }

    // insert your code here
    /*
       generate orthonormal non-exact harmonic 1-forms using Gram-Schmit Orthonormalization,

       a. generate n random 1-forms
       b. Gram-Schmit ortohnormalize them
       c. during the step b, if after the projection, the left 1-form is too small, regenerate a random harmonic 1-form

       the inner product can be carried out using

       CWedgeOperator wp(pM1, pM2); double w = wp.wedge_star_product();
    */
    for (size_t i = 0; i < n; i++)
    {
        M *pM = h_meshes[i];
        g_mapper.set_mesh(pM);
        std::cout << std::endl << "Closed, non-exact Harmonic 1-form " << i << std::endl << std::endl;
        g_mapper.random_harmonic_form();
    }

    std::cout << "After Schmit Orthonormalization " << std::endl;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            CWedgeOperator wp(h_meshes[i], h_meshes[j]);
            std::cout << "(i,j)=" << wp.wedge_star_product() << " ";
        }
        std::cout << std::endl;
    }

    // compute the conjugate harmonic 1-forms using Hodge star operator
    std::vector<M *> t_meshes;
    for (int i = 0; i < n; i++)
    {
        t_meshes.push_back(g_meshes[i]);
    }
    for (int i = 0; i < n; i++)
    {
        t_meshes.push_back(h_meshes[i]);
    }
    CBaseHolomorphicForm holo(t_meshes);
    holo.conjugate();

    // compute the slit map
    std::vector<M *> meshes;
    for (size_t i = 0; i < n; i++)
    {
        meshes.push_back(g_meshes[i]);
    }
    CSlitMap<M> slit_mapper(meshes);
    slit_mapper._slit_map(0, 1);
    // integrate the holomorphic 1-form
    g_mapper.integration(g_meshes[0]);

    // polar map
    polar_map(g_meshes[0]);

    // output the computational result
    using M = CHodgeDecompositionMesh;
    M *pMesh = g_meshes.front();
    for (M::MeshVertexIterator viter(pMesh); !viter.end(); viter++)
    {
        M::CVertex *pv = *viter;
        pv->position() = pv->point();
        pv->point() = CPoint(pv->uv()[0], pv->uv()[1], 0);
    }
    pMesh->write_m(output_mesh_name.c_str());

    for (M::MeshVertexIterator viter(pMesh); !viter.end(); viter++)
    {
        M::CVertex *pv = *viter;
        pv->point() = pv->position();
    }
}
