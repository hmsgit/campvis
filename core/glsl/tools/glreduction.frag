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

#ifdef REDUCTION_1D
    uniform sampler2D _texture;
    uniform ivec2 _textureSize;
#endif

#ifdef REDUCTION_2D
    uniform sampler2D _texture;
    uniform ivec2 _textureSize;
#endif

#ifdef REDUCTION_3D
    uniform sampler3D _texture;
    uniform ivec3 _textureSize;
#endif

void main() {
    ivec2 texel = ivec2(ex_TexCoord.xy * vec2(_textureSize.xy));

#ifdef REDUCTION_1D
    // 2D reduction:
    vec4 v = texelFetch(_texture, ivec2(0, 0), 0);
    for (int x = 1; x < _textureSize.x; x += 1) {
        v = REDUCTION_OP_2(v, texelFetch(_texture, ivec2(x, 0), 0));
    }
#endif

#ifdef REDUCTION_2D
    // 2D reduction:
    vec4 v = texelFetch(_texture, ivec2(texel.x, 0), 0);
    for (int y = 1; y < _textureSize.y; y += 1) {
        v = REDUCTION_OP_2(v, texelFetch(_texture, ivec2(texel.x, y), 0));
    }
#endif

#ifdef REDUCTION_3D
    // 3D reduction along z direction:
    vec4 v = texelFetch(_texture, ivec3(texel.xy, 0), 0);
    for (int z = 1; z < _textureSize.z; z += 1) {
        v = REDUCTION_OP_2(v, texelFetch(_texture, ivec3(texel.xy, z), 0));
    }

#endif

    out_Color = v;
}
