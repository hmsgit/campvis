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

// TODO: implement coordinate transformation using a trafo matrix?

struct TextureParameters3D {
    // Texture size
    vec3 _size;
    vec3 _sizeRCP;

    // Number of channels
    int _numChannels;

    // Voxel spacing
    vec3 _voxelSize;
    vec3 _voxelSizeRCP;

    // Transformation matrices
    mat4 _textureToWorldMatrix;
    mat4 _worldToTextureMatrix;
};

/**
 * Transforms texture coordinates for texture \a tex to world coordinates using the texture's
 * texture-to-world matrix.
 * \param	texParams   TextureParameters3D struct with texture for lookup
 * \param   texCoords   texture coordinates
 * \return  \a texCoords transformes to woorld coordinates.
 */
vec4 textureToWorld(in TextureParameters3D texParams, in vec4 texCoords) {
    return texParams._textureToWorldMatrix * texCoords;
}

/**
 * Transforms texture coordinates for texture \a tex to world coordinates using the texture's
 * texture-to-world matrix.
 * \param	texParams   TextureParameters3D struct with texture for lookup
 * \param   texCoords   texture coordinates
 * \return  \a texCoords transformes to woorld coordinates.
 */
vec4 textureToWorld(in TextureParameters3D texParams, in vec3 texCoords) {
    return textureToWorld(texParams, vec4(texCoords, 1.0));
}

/**
 * Transforms world coordinates for texture \a tex to texture coordinates using the texture's
 * world-to-texture matrix.
 * \param	texParams   TextureParameters3D struct with texture for lookup
 * \param   worldCoords world coordinates
 * \return  \a texCoords transformes to texture coordinates.
 */
vec4 worldToTexture(in TextureParameters3D texParams, in vec4 worldCoords) {
    return texParams._worldToTextureMatrix * worldCoords;
}

/**
 * Transforms world coordinates for texture \a tex to texture coordinates using the texture's
 * world-to-texture matrix.
 * \param	texParams   TextureParameters3D struct with texture for lookup
 * \param   worldCoords world coordinates
 * \return  \a texCoords transformes to texture coordinates.
 */
vec4 worldToTexture(in TextureParameters3D texParams, in vec3 worldCoords) {
    return worldToTexture(texParams, vec4(worldCoords, 1.0));
}
