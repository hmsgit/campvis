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

#include "tools/texture3d.frag"

/**
 * Compute the gradient using forward differences on the texture's red channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientForwardDifferences(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords) {
    float v = texture(tex, texCoords).r;
    float dx = textureOffset(tex, texCoords, ivec3(1, 0, 0)).r;
    float dy = textureOffset(tex, texCoords, ivec3(0, 1, 0)).r;
    float dz = textureOffset(tex, texCoords, ivec3(0, 0, 1)).r;
    return vec3(v - dx, v - dy, v - dz) * texParams._voxelSize;
}

/**
 * Compute the gradient using central differences on the texture's red channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientCentralDifferences(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords) {
    float dx = textureOffset(tex, texCoords, ivec3(1, 0, 0)).r;
    float dy = textureOffset(tex, texCoords, ivec3(0, 1, 0)).r;
    float dz = textureOffset(tex, texCoords, ivec3(0, 0, 1)).r;
    float mdx = textureOffset(tex, texCoords, ivec3(-1, 0, 0)).r;
    float mdy = textureOffset(tex, texCoords, ivec3(0, -1, 0)).r;
    float mdz = textureOffset(tex, texCoords, ivec3(0, 0, -1)).r;
    return vec3(mdx - dx, mdy - dy, mdz - dz) * texParams._voxelSize * 0.5;
}


/**
 * Compute the gradient using filtered central differences on the texture's red channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientFilteredCentralDifferences(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords) {
    vec3 offset = texParams._sizeRCP;
    vec3 g0 = computeGradientCentralDifferences(tex, texParams, texCoords);
    vec3 g1 = computeGradientCentralDifferences(tex, texParams, texCoords + vec3(-offset.x, -offset.y, -offset.z));
    vec3 g2 = computeGradientCentralDifferences(tex, texParams, texCoords + vec3( offset.x,  offset.y,  offset.z));
    vec3 g3 = computeGradientCentralDifferences(tex, texParams, texCoords + vec3(-offset.x,  offset.y, -offset.z));
    vec3 g4 = computeGradientCentralDifferences(tex, texParams, texCoords + vec3( offset.x, -offset.y,  offset.z));
    vec3 g5 = computeGradientCentralDifferences(tex, texParams, texCoords + vec3(-offset.x, -offset.y,  offset.z));
    vec3 g6 = computeGradientCentralDifferences(tex, texParams, texCoords + vec3( offset.x,  offset.y, -offset.z));
    vec3 g7 = computeGradientCentralDifferences(tex, texParams, texCoords + vec3(-offset.x,  offset.y,  offset.z));
    vec3 g8 = computeGradientCentralDifferences(tex, texParams, texCoords + vec3( offset.x, -offset.y, -offset.z));
    vec3 mix0 = mix(mix(g1, g2, 0.5), mix(g3, g4, 0.5), 0.5);
    vec3 mix1 = mix(mix(g5, g6, 0.5), mix(g7, g8, 0.5), 0.5);
    return mix(g0, mix(mix0, mix1, 0.5), 0.75);
}


/**
 * Compute the gradient using the sobel filter on the texture's red channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientSobel(in sampler3D tex, in vec3 texCoords) {
    const float sobelScale = 1.0 / 44.0;
    vec3 sobel = vec3(0.0);
    float value = 0.0;
   
    value = textureOffset(tex, texCoords, ivec3(-1, -1, -1)).r;
    sobel.x -= value; 
    sobel.y -= value; 
    sobel.z -= value; 
    value = textureOffset(tex, texCoords, ivec3( 0, -1, -1)).r;
    sobel.y -= 3.0 * value; 
    sobel.z -= 3.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1, -1, -1)).r;
    sobel.x += value; 
    sobel.y -= value; 
    sobel.z -= value; 
   
    value = textureOffset(tex, texCoords, ivec3(-1,  0, -1)).r;
    sobel.x -= 3.0 * value; 
    sobel.z -= 3.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 0,  0, -1)).r;
    sobel.z -= 6.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1,  0, -1)).r;
    sobel.x += 3.0 * value; 
    sobel.z -= 3.0 * value; 
   
    value = textureOffset(tex, texCoords, ivec3(-1,  1, -1)).r;
    sobel.x -= value; 
    sobel.y += value; 
    sobel.z -= value; 
    value = textureOffset(tex, texCoords, ivec3( 0,  1, -1)).r;
    sobel.y +=  3.0 * value; 
    sobel.z -=  3.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1,  1, -1)).r;
    sobel.x += value; 
    sobel.y += value; 
    sobel.z -= value; 

   
    value = textureOffset(tex, texCoords, ivec3(-1, -1,  0)).r;
    sobel.x -= 3.0 * value; 
    sobel.y -= 3.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 0, -1,  0)).r;
    sobel.y -= 6.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1, -1,  0)).r;
    sobel.x += 3.0 * value; 
    sobel.y -= 3.0 * value; 
   
    value = textureOffset(tex, texCoords, ivec3(-1,  0,  0)).r;
    sobel.x -= 6.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1,  0,  0)).r;
    sobel.x += 6.0 * value; 
   
    value = textureOffset(tex, texCoords, ivec3(-1,  1,  0)).r;
    sobel.x -=  3.0 * value; 
    sobel.y +=  3.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 0,  1,  0)).r;
    sobel.y +=  6.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1,  1,  0)).r;
    sobel.x +=  3.0 * value; 
    sobel.y +=  3.0 * value; 
   
   
    value = textureOffset(tex, texCoords, ivec3(-1, -1,  1)).r;
    sobel.x -= value; 
    sobel.y -= value; 
    sobel.z += value; 
    value = textureOffset(tex, texCoords, ivec3( 0, -1,  1)).r;
    sobel.y -= 3.0 * value; 
    sobel.z += 3.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1, -1,  1)).r;
    sobel.x += value; 
    sobel.y -= value; 
    sobel.z += value; 
   
    value = textureOffset(tex, texCoords, ivec3(-1,  0,  1)).r;
    sobel.x -= 3.0 * value; 
    sobel.z += 3.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 0,  0,  1)).r;
    sobel.z += 6.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1,  0,  1)).r;
    sobel.x += 3.0 * value; 
    sobel.z += 3.0 * value; 
   
    value = textureOffset(tex, texCoords, ivec3(-1,  1,  1)).r;
    sobel.x -= value; 
    sobel.y += value; 
    sobel.z += value; 
    value = textureOffset(tex, texCoords, ivec3( 0,  1,  1)).r;
    sobel.y += 3.0 * value; 
    sobel.z += 3.0 * value; 
    value = textureOffset(tex, texCoords, ivec3( 1,  1,  1)).r;
    sobel.x += value; 
    sobel.y += value; 
    sobel.z += value; 
   
    return -sobelScale * sobel;
}
