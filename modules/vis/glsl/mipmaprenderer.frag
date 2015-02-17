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

in vec3 ex_TexCoord;        ///< incoming texture coordinate
in vec4 ex_Position;        ///< incoming texture coordinate

out uvec4 result;
 
#include "tools/texture2d.frag"

uniform usampler2D _voxelTexture;
uniform int _level; // read from this mipmap level


void main() {
    // compute texel to fetch
    ivec2 coord = ivec2(gl_FragCoord.xy - 0.5) * 2;

    /// Lookup 4 neighbor texels ( ~ voxel stacks)
    uvec4 val1 = texelFetch(_voxelTexture, coord, _level);
    uvec4 val2 = texelFetchOffset(_voxelTexture, coord, _level, ivec2(1, 0));
    uvec4 val3 = texelFetchOffset(_voxelTexture, coord, _level, ivec2(0, 1));
    uvec4 val4 = texelFetchOffset(_voxelTexture, coord, _level, ivec2(1, 1));

    result = val1 | val2 | val3 | val4;
}