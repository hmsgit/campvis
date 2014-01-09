// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

layout (location = 0) in vec3 in_Position;        ///< incoming vertex position
layout (location = 1) in vec3 in_TexCoord;        ///< incoming texture coordinate
layout (location = 2) in vec4 in_Color;           ///< incoming color
layout (location = 3) in vec3 in_Normal;          ///< incoming normals

#ifdef HAS_GEOMETRY_SHADER
out vec4 vert_Position;       ///< outgoing world coordinates
out vec3 vert_TexCoord;       ///< outgoing texture coordinate
out vec4 vert_Color;          ///< outgoing color
out vec3 vert_Normal;         ///< outgoing world normals
#else
out vec4 geom_Position;       ///< outgoing texture coordinate
out vec3 geom_TexCoord;       ///< outgoing color
out vec4 geom_Color;          ///< outgoing world coordinates
out vec3 geom_Normal;         ///< outgoing world normals
#endif


/// Matrix defining model-to-world transformation
uniform mat4 _modelMatrix = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);

/// Matrix defining view transformation
uniform mat4 _viewMatrix = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);

/// Matrix defining projection transformation
uniform mat4 _projectionMatrix = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);


void main() {
#ifdef HAS_GEOMETRY_SHADER

    // set outputs for geometry shader
    vert_Position = vec4(in_Position, 1.0);
    gl_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * vec4(in_Position, 1.0)));

    vert_TexCoord = in_TexCoord;
    vert_Color = in_Color;
    vert_Normal = in_Normal;

#else

    // set outputs for fragment shader
    geom_Position = vec4(in_Position, 1.0);
    gl_Position = _projectionMatrix * (_viewMatrix * (_modelMatrix * vec4(in_Position, 1.0)));

    geom_TexCoord = in_TexCoord;
    geom_Color = in_Color;
    geom_Normal = in_Normal;

#endif
}
