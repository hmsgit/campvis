// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "tools/shading.frag"


in vec3 ex_TexCoord; ///< incoming texture coordinate
in vec4 ex_Position; ///< incoming texture coordinate

out vec4 out_Color; ///< outgoing fragment color

uniform vec4 _color;
uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

void main() {
out_Color = _color;

#ifdef ENABLE_SHADING
// compute gradient (needed for shading and normals)
vec3 gradient = ex_TexCoord;
out_Color.rgb = calculatePhongShading(ex_Position.xyz / ex_Position.z, _lightSource, _cameraPosition, gradient, _color.rgb, _color.rgb, vec3(1.0, 1.0, 1.0));
#endif
}
