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

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/texture2d.frag"

#ifdef REDUCTION_2D
    uniform sampler2D _texture;
    uniform vec2 _texCoordsShift;
#endif

#ifdef REDUCTION_3D
    uniform sampler3D _texture;
    uniform vec2 _texCoordsShift;
    uniform int _textureDepth;
#endif

void main() {
    vec2 tmp = ex_TexCoord.xy - _texCoordsShift;

#ifdef REDUCTION_2D
    // 2D reduction:
    vec4 a = texture(_texture, tmp);
    vec4 b = textureOffset(_texture, tmp, ivec2(1, 0));
    vec4 c = textureOffset(_texture, tmp, ivec2(0, 1));
    vec4 d = textureOffset(_texture, tmp, ivec2(1, 1));
#endif

#ifdef REDUCTION_3D
    // 3D reduction along depth:
    float textureDepthRCP = 1.0 / _textureDepth;

    vec4 a = texture(_texture, vec3(tmp, textureDepthRCP/2.0));
    vec4 b = textureOffset(_texture, vec3(tmp, textureDepthRCP/2.0), ivec3(1, 0, 0));
    vec4 c = textureOffset(_texture, vec3(tmp, textureDepthRCP/2.0), ivec3(0, 1, 0));
    vec4 d = textureOffset(_texture, vec3(tmp, textureDepthRCP/2.0), ivec3(1, 1, 0));

    for (float z = 3.0 * textureDepthRCP / 2.0; z < 1.0; z += textureDepthRCP) {
        a = REDUCTION_OP_2(a, texture(_texture, vec3(tmp, z)));
        b = REDUCTION_OP_2(b, textureOffset(_texture, vec3(tmp, z), ivec3(1, 0, 0)));
        c = REDUCTION_OP_2(c, textureOffset(_texture, vec3(tmp, z), ivec3(0, 1, 0)));
        d = REDUCTION_OP_2(d, textureOffset(_texture, vec3(tmp, z), ivec3(1, 1, 0)));
    }
#endif

    // final reduction of the four pixels
    out_Color = REDUCTION_OP_4(a, b, c, d);
}
