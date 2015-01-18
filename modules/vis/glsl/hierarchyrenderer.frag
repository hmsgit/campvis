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
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

// DRR volume
uniform sampler3D _volume;
uniform TextureParameters3D _volumeTextureParams;

// Transfer function
uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform float _inverseTexSizeX; // 1.0 / mipmapLevelResolution
uniform float _inverseTexSizeY; // 1.0 / mipmapLevelResolution

uniform int _brickSize;
uniform int _brickDepth;
uniform vec3 _hierarchySize;
uniform vec2 _tfDomain;

void main() {
    result = uvec4(0);

    // For each element in the uvec4 bitmask:
    for (int e = 0; e < 4; ++e) {
        // For each bit of the uint value, the corresponding area in the volume is checked, whether it's transparent or not. The bits are set accordingly.
        for (int d = 0; d < 32; d++) {
            vec3 llf = vec3(gl_FragCoord.xy / _hierarchySize.xy, float((e * 32) + d + 0.5) / 128.0);
            //ivec3 llf = ivec3((gl_FragCoord.x - 0.5) * _brickSize, (gl_FragCoord.y - 0.5) * _brickSize, );
            bool hasData = false;

            /** 
             * For each bit _brickSize number of voxels in x and y direction and _brickDepth number of voxel in z-direction should be considered.
             * Also, to make sure that all voxels are considered, one offset voxel is considered in x, y, and z boundary from each side so that
             * each side will consider 2 more voxels.
             */
            for (int z = -1; z < _brickDepth + 1; ++z) {
                for (int y = -1; y < _brickSize + 1; ++y) {
                    for (int x = -1; x < _brickSize + 1; ++x) {
                        vec3 addendum = (vec3(x, y, z) / _volumeTextureParams._size);
                        vec3 texCoord = clamp(llf + addendum, 0.0, 1.0);
                        //float intensity = mapIntensityToTFDomain(_transferFunctionParams._intensityDomain, texture(_volume, texCoord).r);
                        ivec3 voxel = ivec3(texCoord * _volumeTextureParams._size);
                        float intensity = mapIntensityToTFDomain(_transferFunctionParams._intensityDomain, texelFetch(_volume, voxel, 0).r);
                        //float intensity = texture(_volume, voxel).r;
                        //vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);
                        //if (color.a > 0) {
                        //}

                        // if there was any data in the volume data in that voxel, set the bit.
                        if (intensity >= _tfDomain.x && intensity <= _tfDomain.y) {
                            //result[e] |= (1 << d);
                            if (e == 0)
                                result.r |= (1 << d);
                            else if (e == 1)
                                result.g |= (1 << d);
                            else if (e == 2)
                                result.b |= (1 << d);
                            else if (e == 3)
                                result.a |= (1 << d);
                            break;
                        }
                    }
                }
            }
        }
    }
}