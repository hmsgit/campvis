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

subroutine void ComputationRoutineType();
subroutine uniform ComputationRoutineType ComputationRoutine;

in vec3 ex_TexCoord;        ///< incoming texture coordinate
in vec4 ex_Position;        ///< incoming texture coordinate

out uint result;
 
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

uniform uint _voxelSize;
uniform uint _voxelDepth;

void main() {
    result = uint(0);

    vec3 startIdx = vec3(0, 0, 0);
    vec3 endIdx = _volumeTextureParams._size - vec3(1, 1, 1);

    // For each bit of the int value, the area is checked to find voxels or not. If some voxels were found, the bit is set or cleared.
    for (int d = 0; d < 32; d++) {
        vec3 samplePosition = vec3((gl_FragCoord.x - 0.5) * _voxelSize, (gl_FragCoord.y - 0.5) * _voxelSize, d * _voxelDepth);

        bool hasData = false;
        float intensity;
        vec4 color;

        /** For each bit _voxelSize number of voxels in x and y direction and _voxelDepth number of voxel in z-direction should be considered.
         * Also, to make sure that all voxels are considered, one offset voxel is considered in x, y, and z boundary from each side so that
         * each side will consider 2 more voxels.
         */
        for (int i = 0; i < (_voxelSize + 2) * (_voxelSize + 2) * (_voxelDepth + 2); i++) {

            // the offset value calculates which voxel should be checked.
            ivec3 offset = ivec3(i % (_voxelSize + 2) - 1, (i / (_voxelSize + 2)) % (_voxelSize + 2) - 1, i / ((_voxelSize + 2) * (_voxelSize + 2)) - 1);
            
            vec3 s = clamp(samplePosition + vec3(0.5, 0.5, 0.5) + offset, startIdx, endIdx);
            intensity = texture(_volume, s * _volumeTextureParams._sizeRCP).r;
            color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

            // if there was any data in the volume data in that voxel, set the bit.
            if (color.a > 0.0) {
                hasData = true;
                break;
            }
        }

        if (hasData) {
            result |= (1 << d);
        }
    }
}