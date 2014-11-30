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

out vec3 ex_TexCoord;       ///< outgoing texture coordinate

/// Matrix defining projection transformation
uniform mat4 _projectionMatrix = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);

// Parameters defining fan geometry
uniform float halfAngle;
uniform float innerRadius;


void main() {
    float radius = innerRadius + (1 - innerRadius) * in_Position.y;
    vec4 pos = vec4(sin(halfAngle * in_Position.x*2) * radius,
                    cos(halfAngle * in_Position.x*2) * radius,
                    in_Position.z, 1);

    gl_Position = _projectionMatrix * pos;
    ex_TexCoord = in_TexCoord;
}
