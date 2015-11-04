// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "tools/texture3d.frag"

/**
 * Compute the gradient using forward differences on the texture's red channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texParams   Texture parameters struct (needs texParams._voxelSize)
 * \param   texCoords   Lookup position in texture coordinates
 * \param   lod         Explicit LOD in texture to look up
 */
vec3 computeGradientForwardDifferencesLod(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords, in float lod) {
    float v = textureLod(tex, texCoords, lod).r;
    float dx = textureLodOffset(tex, texCoords, lod, ivec3(1, 0, 0)).r;
    float dy = textureLodOffset(tex, texCoords, lod, ivec3(0, 1, 0)).r;
    float dz = textureLodOffset(tex, texCoords, lod, ivec3(0, 0, 1)).r;
    return vec3(dx - v, dy - v, dz - v) * texParams._voxelSize;
}

/**
 * Compute the gradient using forward differences on the texture's red channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texParams   Texture parameters struct (needs texParams._voxelSize)
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientForwardDifferences(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords) {
    return computeGradientForwardDifferencesLod(tex, texParams, texCoords, 0.0);
}

/**
 * Compute the gradient using central differences on the texture's red channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texParams   Texture parameters struct (needs texParams._voxelSize)
 * \param   texCoords   Lookup position in texture coordinates
 * \param   lod         Explicit LOD in texture to look up
 */
vec3 computeGradientCentralDifferencesLod(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords, in float lod) {
    float dx = textureLodOffset(tex, texCoords, lod, ivec3(1, 0, 0)).r;
    float dy = textureLodOffset(tex, texCoords, lod, ivec3(0, 1, 0)).r;
    float dz = textureLodOffset(tex, texCoords, lod, ivec3(0, 0, 1)).r;
    float mdx = textureLodOffset(tex, texCoords, lod, ivec3(-1, 0, 0)).r;
    float mdy = textureLodOffset(tex, texCoords, lod, ivec3(0, -1, 0)).r;
    float mdz = textureLodOffset(tex, texCoords, lod, ivec3(0, 0, -1)).r;
    return vec3(dx - mdx, dy - mdy, dz - mdz) * texParams._voxelSize * 0.5;
}

/**
 * Compute the gradient using central differences on the texture's red channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texParams   Texture parameters struct (needs texParams._voxelSize)
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientCentralDifferences(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords) {
    return computeGradientCentralDifferencesLod(tex, texParams, texCoords, 0.0);
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
   
    return sobelScale * sobel;
}


/**
 * Compute the Hessian matrix of the input image using central differences twice.
 * \param   tex         3D gradient texture to calculate Hessian for
 * \param   texCoords   Lookup position in texture coordinates
 */
mat3 computeHessianCentralDifferencesLod(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords, in float lod) {
    vec3 offset = texParams._sizeRCP * pow(2.0, lod);

    // compute first derivatives using central differences
    vec3 dx  = computeGradientCentralDifferencesLod(tex, texParams, texCoords + vec3(offset.x, 0.0, 0.0), lod);
    vec3 dy  = computeGradientCentralDifferencesLod(tex, texParams, texCoords + vec3(0.0, offset.y, 0.0), lod);
    vec3 dz  = computeGradientCentralDifferencesLod(tex, texParams, texCoords + vec3(0.0, 0.0, offset.z), lod);
    vec3 mdx = computeGradientCentralDifferencesLod(tex, texParams, texCoords - vec3(offset.x, 0.0, 0.0), lod);
    vec3 mdy = computeGradientCentralDifferencesLod(tex, texParams, texCoords - vec3(0.0, offset.y, 0.0), lod);
    vec3 mdz = computeGradientCentralDifferencesLod(tex, texParams, texCoords - vec3(0.0, 0.0, offset.z), lod);

    // compute hessian matrix columns (second order derivatives) using central differences of gradients
    vec3 vdx = (dx - mdx) * texParams._voxelSize * 0.5;
    vec3 vdy = (dy - mdy) * texParams._voxelSize * 0.5;
    vec3 vdz = (dz - mdz) * texParams._voxelSize * 0.5;

    return mat3(vdx, vdy, vdz);
}

/**
 * Compute the Hessian matrix using central differences on the gradient texture's RGB channel.
 * \param   tex         3D gradient texture to calculate Hessian for
 * \param   texCoords   Lookup position in texture coordinates
 */
mat3 computeGradientHessianCentralDifferencesLod(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords, in float lod) {
    vec3 offset = texParams._sizeRCP * pow(2.0, lod);

    vec3 dx = textureLod(tex, texCoords + vec3(offset.x, 0.0, 0.0), lod).rgb;
    vec3 dy = textureLod(tex, texCoords + vec3(0.0, offset.y, 0.0), lod).rgb;
    vec3 dz = textureLod(tex, texCoords + vec3(0.0, 0.0, offset.z), lod).rgb;
    vec3 mdx = textureLod(tex, texCoords - vec3(offset.x, 0.0, 0.0), lod).rgb;
    vec3 mdy = textureLod(tex, texCoords - vec3(0.0, offset.y, 0.0), lod).rgb;
    vec3 mdz = textureLod(tex, texCoords - vec3(0.0, 0.0, offset.z), lod).rgb;

    float dxx = dx.x - mdx.x;
    float dxy = dx.y - mdx.y;
    float dxz = dx.z - mdx.z;

    float dyx = dy.x - mdy.x;
    float dyy = dy.y - mdy.y;
    float dyz = dy.z - mdy.z;

    float dzx = dz.x - mdz.x;
    float dzy = dz.y - mdz.y;
    float dzz = dz.z - mdz.z;

    mat3 hessian = mat3(dxx, (dyx + dxy) / 2.0, (dzx + dxz) / 2.0,
                        (dxy + dyx) / 2.0, dyy, (dzy + dyz) / 2.0,
                        (dxz + dzx) / 2.0, (dyz + dzy) / 2.0, dzz);
    //hessian *= (texParams._voxelSize * 0.5;
    return hessian;
}


/**
 * Compute the Hessian matrix using central differences on the gradient texture's RGB channel.
 * \param   tex         3D gradient texture to calculate Hessian for
 * \param   texCoords   Lookup position in texture coordinates
 */
mat3 computeHessianCentralDifferences(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords) {
    return computeHessianCentralDifferencesLod(tex, texParams, texCoords, 0.0);
}

// code inspiured by http://en.wikipedia.org/wiki/Eigenvalue_algorithm#3.C3.973_matrices
vec3 computeEigenvalues(in mat3 hessian) {
    const float PI = 3.1415926535897932384626433832795;
    vec3 toReturn;
    float p1 = (hessian[1][0] * hessian[1][0]) + (hessian[2][0] * hessian[2][0]) + (hessian[2][1] * hessian[2][1]);

    if (p1 == 0.0) {
        toReturn.x = hessian[0][0];
        toReturn.y = hessian[1][1];
        toReturn.z = hessian[2][2];
    }
    else {
        float q = (hessian[0][0] + hessian[1][1] + hessian[2][2]) / 3.0;
        float p2 = (hessian[0][0] - q) * (hessian[0][0] - q)   +   (hessian[1][1] - q) * (hessian[1][1] - q)   +   (hessian[2][2] - q) * (hessian[2][2] - q)   +   2.0 * p1;
        float p = sqrt(p2 / 6.0);
        mat3 B = (1.0 - p) * (hessian - mat3(q));
        float r = determinant(B) / 2.0;

        float phi = acos(clamp(r, -1.0, 1.0)) / 3.0;

        toReturn.x = q + (2.0 * p * cos(phi));
        toReturn.z = q + (2.0 * p * cos(phi + (2.0 * PI / 3.0)));
        toReturn.y = 3.0 * q - toReturn.x - toReturn.z;
    }

    return toReturn;
}