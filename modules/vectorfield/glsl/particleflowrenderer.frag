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
#include "tools/transferfunction.frag"

in vec3 ex_Direction;
in float ex_Transparency;

out vec4 out_Color; ///< outgoing fragment color

uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;
uniform int _coloringMode;
uniform float _scale;
uniform vec2 _threshold;

void main() {
    if (length(gl_PointCoord - vec2(0.5)) > 0.5) {
        discard;
        }

    switch (_coloringMode) {
        case 0: // age
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, ex_Transparency);
            break;
        case 1: // velocity
            float f = length(ex_Direction);
            f -= _threshold.x;
            f /= (_threshold.y - _threshold.x);
            f *= _scale;
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, f);
            break;
        case 2: // direction
            out_Color = vec4(normalize(abs(ex_Direction)), 1.0);
            break;
        default:
            discard;
    }

#ifdef ENABLE_SHADING___
    // compute gradient (needed for shading and normals)
    vec3 gradient = ex_Normal;
    out_Color.rgb = calculatePhongShading(ex_Position, _lightSource, _cameraPosition, gradient, _color.rgb, _color.rgb, vec3(1.0, 1.0, 1.0));
#endif

}
