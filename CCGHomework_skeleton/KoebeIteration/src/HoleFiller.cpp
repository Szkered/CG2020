
#include "HoleFiller.h"

using namespace MeshLib;

/*! Suppose the input mesh is a circular slit map
 *   fill the center hole
 */
void MeshLib::fill_hole(CDTMesh &mesh, std::string &output_filled_mesh)
{

    // CDTMesh mesh;
    CDTMesh omesh;

    // take the boundary of the input mesh
    CDTMesh::CBoundary bnd(&mesh);
    // take the center hole loop
    CDTMesh::CLoop *pL = bnd.loops()[1];
    // get the ordered list of half edges of the hole boundary
    std::list<CDTMesh::tHalfEdge> hs = pL->halfedges();

    // the vertices on the hole boundary
    std::vector<CDTMesh::CVertex *> verts;
    // the points on the hole boundary
    std::vector<CPoint2> points;

    // take the hole boundary
    for (std::list<CDTMesh::tHalfEdge>::iterator hiter = hs.begin(); hiter != hs.end(); hiter++)
    {
        CDTMesh::tHalfEdge ph = *hiter;
        CDTMesh::CVertex *pv = mesh.halfedgeTarget(ph);
        CPoint p = pv->point();
        points.push_back(CPoint2(p[0], p[1]));
        verts.push_back(pv);
        CDTMesh::CEdge *pe = mesh.halfedgeEdge(ph);
        pe->string() = std::string("sharp");
    }

    // generate a new mesh covering the hole using Delaunay algorithm
    // the boundary of the new mesh is the above point list
    CDTMesh tmesh;
    tmesh.GenerateMesh(points, omesh);

    // build a map between the boundary of the new mesh
    // and the hole boundary of the input mesh
    std::map<CDTMesh::CVertex *, CDTMesh::CVertex *> vmap;
    size_t i = 0;
    for (CDTMesh::MeshVertexIterator viter(&omesh); !viter.end(); viter++)
    {
        CDTMesh::CVertex *pv = *viter;
        CDTMesh::CVertex *pw = verts[i++];
        vmap.insert(std::pair<CDTMesh::CVertex *, CDTMesh::CVertex *>(pv, pw));
        if (i == verts.size())
            break;
    }

    // find the maximal vertex id of the input mesh
    int max_vid = 0;
    for (CDTMesh::MeshVertexIterator viter(&mesh); !viter.end(); viter++)
    {
        CDTMesh::CVertex *pv = *viter;
        max_vid = (max_vid > pv->id()) ? max_vid : pv->id();
    }
    // find the maximal face id of the input mesh
    int max_fid = 0;
    for (CDTMesh::MeshFaceIterator fiter(&mesh); !fiter.end(); fiter++)
    {
        CDTMesh::CFace *pf = *fiter;
        max_fid = (max_fid > pf->id()) ? max_fid : pf->id();
    }

    // copy new vertices from the hole_covering new mesh to the original input mesh
    // using create vertex method
    for (CDTMesh::MeshVertexIterator viter(&omesh); !viter.end(); viter++)
    {
        CDTMesh::CVertex *pv = *viter;
        CDTMesh::CVertex *pw = NULL;
        if (pv->boundary())
        {
            pw = vmap[pv];
            pv->idx() = pw->id();
        }
        else
        {
            pv->idx() = pv->id() + max_vid;
            pw = mesh.createVertex(pv->idx());
            pw->point() = pv->point();
            pw->string() = std::string("rgb=(1 0 0) normal=(0 0 1)");
            pw->normal() = CPoint(0, 0, 1);
        }
    }

    // copy faces from hole covering mesh to the original mesh
    for (CDTMesh::MeshFaceIterator fiter(&omesh); !fiter.end(); fiter++)
    {
        CDTMesh::CFace *pf = *fiter;
        std::vector<CDTMesh::CVertex *> vts;

        for (CDTMesh::FaceVertexIterator vfiter(pf); !vfiter.end(); vfiter++)
        {
            CDTMesh::CVertex *pv = *vfiter;
            int idx = pv->idx();
            CDTMesh::CVertex *pw = mesh.idVertex(idx);
            vts.push_back(pw);
        }
        mesh.createFace(vts, pf->id() + max_fid);
    }

    // output the mesh with the filled hole
    mesh.write_m(output_filled_mesh.c_str());
}

/*!
 *   punch a hole from the filled mesh, which corresponds to the id-th hole in the original mesh
 */
void MeshLib::punch_hole(CDTMesh &original_mesh, CDTMesh &filled_mesh, std::string &punched_mesh_name, int id)
{
    // take the boundary of the original mesh
    CDTMesh::CBoundary cnd(&original_mesh);
    // take the id-th hole of the original mesh
    CDTMesh::CLoop *pL = cnd.loops()[id];
    // take the ordered list of halfedges of the hole
    std::list<CDTMesh::CHalfEdge *> &hs = pL->halfedges();

    // initialize the vertices
    for (CDTMesh::MeshVertexIterator viter(&filled_mesh); !viter.end(); viter++)
    {
        CDTMesh::CVertex *pv = *viter;
        pv->touched() = false;
    }
    // initialize the edges
    for (CDTMesh::MeshEdgeIterator eiter(&filled_mesh); !eiter.end(); eiter++)
    {
        CDTMesh::CEdge *pe = *eiter;
        pe->frozen() = false;
    }

    // flooding algorithm
    std::queue<CDTMesh::CVertex *> queue;
    // find the edges on the filled_mesh corresponding to the hole boundary of the original mesh
    // mark these edges as frozen, find the vertices inside the frozen edges
    for (std::list<CDTMesh::CHalfEdge *>::iterator hiter = hs.begin(); hiter != hs.end(); hiter++)
    {
        CDTMesh::CHalfEdge *ph = *hiter;

        CDTMesh::CVertex *ps = original_mesh.halfedgeSource(ph);
        CDTMesh::CVertex *pt = original_mesh.halfedgeTarget(ph);

        CDTMesh::CVertex *pWs = filled_mesh.idVertex(ps->id());
        CDTMesh::CVertex *pWt = filled_mesh.idVertex(pt->id());

        CDTMesh::CEdge *pe = filled_mesh.vertexEdge(pWs, pWt);
        pe->frozen() = true;

        CDTMesh::CHalfEdge *pwh = filled_mesh.edgeHalfedge(pe, 0);
        if (filled_mesh.halfedgeTarget(pwh) == pWt)
        {
            pwh = filled_mesh.halfedgeSym(pwh);
        }

        pwh = filled_mesh.halfedgeNext(pwh);
        CDTMesh::CVertex *pw = filled_mesh.halfedgeTarget(pwh);
        pw->touched() = true;
        queue.push(pw);
    }

    // insert your code
    // flood into the interior of the frozen loop
    // label all the vertices as touched
    while (!queue.empty())
    {
        CDTMesh::CVertex *pv = queue.front();
        queue.pop();
    }

    // generate a new mesh, ignoring all the touched vertices
    // and ignoring the faces, which includes any touched vertices

    CDTMesh punched_mesh;
    for (CDTMesh::MeshVertexIterator viter(&filled_mesh); !viter.end(); viter++)
    {
        CDTMesh::CVertex *pv = *viter;

        // insert your code here
        // ignore the touched vertex

        CDTMesh::CVertex *pw = punched_mesh.createVertex(pv->id());
        pw->string() = pv->string();
        pw->point() = pv->point();
    }

    for (CDTMesh::MeshFaceIterator fiter(&filled_mesh); !fiter.end(); fiter++)
    {
        CDTMesh::CFace *pf = *fiter;

        // insert your code here
        // ignore any face, which has any touched vertex
        bool valid = true;
        if (!valid)
            continue;

        // take the vertices of the face
        std::vector<CDTMesh::CVertex *> vts;
        for (CDTMesh::FaceVertexIterator fviter(pf); !fviter.end(); fviter++)
        {
            CDTMesh::CVertex *pv = *fviter;
            vts.push_back(pv);
        }

        // find the corresponding vertices on the new mesh
        std::vector<CDTMesh::CVertex *> wts;
        for (size_t i = 0; i < vts.size(); i++)
        {
            CDTMesh::CVertex *pv = vts[i];
            CDTMesh::CVertex *pw = punched_mesh.idVertex(pv->id());
            wts.push_back(pw);
        }

        CDTMesh::CFace *pwf = punched_mesh.createFace(wts, pf->id());
        pwf->string() = pf->string();
    }
    // output the punched mesh
    punched_mesh.write_m(punched_mesh_name.c_str());
}
