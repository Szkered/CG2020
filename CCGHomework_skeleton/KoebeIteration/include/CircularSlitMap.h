#ifndef _CIRCULAR_SLIT_MAP_H_
#define _CIRCULAR_SLIT_MAP_H_

#include <vector>
#include <list>
#include "HodgeDecomposition.h"
#include "HodgeDecompositionMesh.h"
#include "WedgeProduct.h"
#include "SlitMap.h"
#include <vector>

namespace MeshLib
{
void normalizeMesh(CHodgeDecompositionMesh *pMesh);
void computeNormal(CHodgeDecompositionMesh *pMesh);
void polar_map(CHodgeDecompositionMesh *pMesh);

void calc_holo_1_form_closed_mesh(const std::string &input_mesh);
void calc_holo_1_form_open_mesh(const std::string &input_mesh,
                                std::vector<CHodgeDecompositionMesh *> &g_meshes, // exact harmonic 1-forms
                                std::vector<CHodgeDecompositionMesh *> &h_meshes, // closed harmonic 1-forms
                                std::string &output_mesh_name);
} // namespace MeshLib
#endif //_CIRCULAR_SLIT_MAP_H_
