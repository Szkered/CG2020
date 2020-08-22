#ifndef _HOLE_FILLER_H_
#define _HOLE_FILLER_H_

#include <vector>
#include <list>
#include "DelaunayMesh.h"

namespace MeshLib
{

void fill_hole(CDTMesh &original_mesh, std::string &output_filled_mesh);
void punch_hole(CDTMesh &original_mesh, CDTMesh &filled_mesh, std::string &punched_mesh_name, int id);

} // namespace MeshLib
#endif // !_HOLE_FILLER_H_
