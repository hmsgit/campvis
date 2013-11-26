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

out uint result;
 
#include "tools/texture2d.frag"

uniform usampler2D _voxelTexture;
uniform TextureParameters2D _voxelTextureParams;
uniform int _level; // read from this mipmap level

uniform float _inverseTexSizeX; // 1.0 / mipmapLevelResolution
uniform float _inverseTexSizeY; // 1.0 / mipmapLevelResolution


void main() {
    // texture coordinates of 4 neighbor texels in source voxel texture
    vec2 offset1 = vec2(_inverseTexSizeX, 0.0);   // right
    vec2 offset2 = vec2(0.0, _inverseTexSizeY);   // top
    vec2 offset3 = vec2(_inverseTexSizeX, _inverseTexSizeY); // top right
    vec2 coord; // this pixel    
    coord.x = (((gl_FragCoord.x-0.5)*2.0)+0.5)*_inverseTexSizeX;
    coord.y = (((gl_FragCoord.y-0.5)*2.0)+0.5)*_inverseTexSizeY;

    /// Lookup 4 neighbor texels ( ~ voxel stacks)
    uint val1 = uint(textureLod(_voxelTexture, vec2(coord), _level));
    uint val2 = uint(textureLod(_voxelTexture, vec2(coord+offset1), _level) );
    uint val3 = uint(textureLod(_voxelTexture, vec2(coord+offset2), _level) );
    uint val4 = uint(textureLod(_voxelTexture, vec2(coord+offset3), _level) );

    result = val1 | val2 | val3 | val4;
}