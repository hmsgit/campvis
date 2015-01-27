// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

in vec3 in_Position;        ///< incoming vertex position
in vec3 in_TexCoord;        ///< incoming texture coordinate
in vec4 in_Color;           ///< incoming color

out vec3 vert_TexCoord;       ///< outgoing texture coordinate
out vec4 vert_Position;       ///< outgoing world coordinates
out vec4 vert_Color;          ///< outgoing color

uniform int _coloringMode = 0;
uniform vec3 _apex;
uniform vec3 _base;
uniform vec3 _cameraPosition;



vec4 applyMModeColoring(float projection) {
    if (projection > 0) 
        return mix(vec4(0.8, 0.8, 0.8, 1.0), vec4(1.0, 0.0, 0.0, 1.0), smoothstep(0.0, 1.0, projection));
    else
        return mix(vec4(0.0, 0.0, 1.0, 1.0), vec4(0.8, 0.8, 0.8, 1.0), smoothstep(-1.0, 0.0, projection));
}

void main() {
    vert_Color = vec4(1.0, 0.5, 0.0, 1.0);

    if (_coloringMode == 0) {
        vert_Color = vec4(abs(in_TexCoord), 1.0);
        float div = max(vert_Color.r, max(vert_Color.g, vert_Color.b));
        if (div > 1.0)
            vert_Color /= div;
    }
    else if (_coloringMode == 1) {
        vec3 ray = normalize(in_Position.xyz - _cameraPosition);
        float projection = dot(normalize(in_TexCoord), ray);
        vert_Color = applyMModeColoring(projection);
    }
    else if (_coloringMode == 2) {
        vec3 centerline = _apex - _base;
        float d = dot(in_Position - _base, centerline) / dot(centerline, centerline);
        vec3 q = _base + (d * centerline);

        vec3 ray = normalize(q - in_Position);
        float projection = dot(normalize(in_TexCoord), ray);
        vert_Color = applyMModeColoring(projection);
    }

    gl_Position = vert_Position;
    vert_Position = vec4(in_Position, 1.0);
    vert_TexCoord = in_TexCoord;
}
