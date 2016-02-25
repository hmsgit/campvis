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

layout(location = 0) out vec4 out_Color;     ///< outgoing fragment color
layout(location = 1) out vec4 out_FHP;       ///< outgoing fragment first hitpoint
layout(location = 2) out vec4 out_FHN;       ///< outgoing fragment first hit normal

#include "tools/gradient.frag"
#include "tools/raycasting.frag"
#include "tools/shading.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"
#include "modules/vis/glsl/voxelhierarchy.frag"

uniform vec2 _viewportSizeRCP;
uniform float _jitterStepSizeMultiplier;


// ray entry points
uniform sampler2D _entryPoints;
uniform sampler2D _entryPointsDepth;
uniform TextureParameters2D _entryParams;

// ray exit points
uniform sampler2D _exitPoints;
uniform sampler2D _exitPointsDepth;
uniform TextureParameters2D _exitParams;

uniform usampler3D _indexTexture;
uniform sampler3D _lodTexture;
uniform sampler3D _gradientTexture;
uniform TextureParameters3D _volumeTextureParams;

uniform vec3 _indexTextureSize;
uniform vec3 _lodTextureSize;
uniform vec3 _nonNpotVolumeCompensationMultiplier;

// Transfer function
uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;


uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;

#ifdef ENABLE_SHADOWING
uniform float _shadowIntensity;
#endif

const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

struct vec8 {
    vec4 intensity;
    vec4 gradient;
};

vec8 performIntrablockSampling(in vec4 blockInfo, in vec3 intrablockTexCoords) {
    vec3 lodTexCoord = (blockInfo.xyz + clamp(blockInfo.w * intrablockTexCoords, 0.5, blockInfo.w - 0.5)) / _lodTextureSize;
    vec8 result;
    result.intensity = texture(_lodTexture, lodTexCoord);
#ifdef ENABLE_SHADING
    result.gradient = texture(_gradientTexture, lodTexCoord);
#endif
    return result;
}

vec8 loookupIntensity(in vec3 samplePosition) {
    // convert volume texture coordinates to index texture texel coordinates
    vec3 indexLookupPosition = (samplePosition * _indexTextureSize) / _nonNpotVolumeCompensationMultiplier;

    // the texel is defined by the integer part
    ivec3 indexTexel = ivec3(indexLookupPosition);
    // the position within the block is defined by the fractional part
    vec3 blockSamplePosition = fract(indexLookupPosition);

    // the following implementation for trilinear interpolation between blocks of different LODs
    // is mostly inspired by "Multiresolution Interblock Interpolation in Direct Volume Rendering"
    // by Ljung et al.

    // Step 1: Determine the 8 block neighborhood
    ivec3 rst0 = ivec3(floor(clamp(indexLookupPosition - 0.5, vec3(0.0), _indexTextureSize - 1.0)));
    vec3 rst = (indexLookupPosition - vec3(rst0) - 1.0);
    
    // Step 1.5: Gather block information from the local 8-neighborhood
    vec4 blockInfos[8];
    blockInfos[0] = texelFetch(_indexTexture, rst0 + ivec3(0, 0, 0), 0);
    blockInfos[1] = texelFetch(_indexTexture, rst0 + ivec3(1, 0, 0), 0);
    blockInfos[2] = texelFetch(_indexTexture, rst0 + ivec3(0, 1, 0), 0);
    blockInfos[3] = texelFetch(_indexTexture, rst0 + ivec3(1, 1, 0), 0);
    blockInfos[4] = texelFetch(_indexTexture, rst0 + ivec3(0, 0, 1), 0);
    blockInfos[5] = texelFetch(_indexTexture, rst0 + ivec3(1, 0, 1), 0);
    blockInfos[6] = texelFetch(_indexTexture, rst0 + ivec3(0, 1, 1), 0);
    blockInfos[7] = texelFetch(_indexTexture, rst0 + ivec3(1, 1, 1), 0);

    // Step 2: Fetch samples from each of the blocks using intrablock sampling
    vec8 phis[8];
    phis[0] = performIntrablockSampling(blockInfos[0], rst + vec3(1, 1, 1));
    phis[1] = performIntrablockSampling(blockInfos[1], rst + vec3(0, 1, 1));
    phis[2] = performIntrablockSampling(blockInfos[2], rst + vec3(1, 0, 1));
    phis[3] = performIntrablockSampling(blockInfos[3], rst + vec3(0, 0, 1));
    phis[4] = performIntrablockSampling(blockInfos[4], rst + vec3(1, 1, 0));
    phis[5] = performIntrablockSampling(blockInfos[5], rst + vec3(0, 1, 0));
    phis[6] = performIntrablockSampling(blockInfos[6], rst + vec3(1, 0, 0));
    phis[7] = performIntrablockSampling(blockInfos[7], rst + vec3(0, 0, 0));

    // Step 3: Compute edge weights
    for (int i = 0; i < 8; ++i)
        blockInfos[i] = 1 / (blockInfos[i]);

    #define EDGE(II, JJ, RST) clamp((RST + blockInfos[II].w)/(blockInfos[II].w + blockInfos[JJ].w), 0.0, 1.0)
    float e01 = EDGE(0, 1, rst.x);
    float e23 = EDGE(2, 3, rst.x);
    float e45 = EDGE(4, 5, rst.x);
    float e67 = EDGE(6, 7, rst.x);

    float e02 = EDGE(0, 2, rst.y);
    float e13 = EDGE(1, 3, rst.y);
    float e46 = EDGE(4, 6, rst.y);
    float e57 = EDGE(5, 7, rst.y);

    float e04 = EDGE(0, 4, rst.z);
    float e15 = EDGE(1, 5, rst.z);
    float e26 = EDGE(2, 6, rst.z);
    float e37 = EDGE(3, 7, rst.z);

    // Step 4: Compute block weights:
    float omegas[8];
    omegas[0] = (1 - e01) * (1 - e02) * (1 - e04);
    omegas[1] = (    e01) * (1 - e13) * (1 - e15);
    omegas[2] = (1 - e23) * (    e02) * (1 - e26);
    omegas[3] = (    e23) * (    e13) * (1 - e37);
    omegas[4] = (1 - e45) * (1 - e46) * (    e04);
    omegas[5] = (    e45) * (1 - e57) * (    e15);
    omegas[6] = (1 - e67) * (    e46) * (    e26);
    omegas[7] = (    e67) * (    e57) * (    e37);

    // Step 5: Compute normalized weighted sum:
    vec4 intensities = vec4(0.0);
    vec4 gradients = vec4(0.0);
    float unten = 0.0;
    for (int i = 0; i < 8; ++i) {
        intensities += phis[i].intensity * omegas[i];
        gradients += phis[i].gradient * omegas[i];
        unten += omegas[i];
    }

    vec8 result;
    result.intensity = intensities/unten;
    result.gradient = gradients/unten;

    return result;

    //// Get the block from the index texture:
    ////  - The first three components define the texel within the lodTexture
    ////  - The fourth component defines the corresponding block size
    //vec4 blockInfo = vec4(texelFetch(_indexTexture, indexTexel, 0));
    //
    //vec3 lodTexCoord = (blockInfo.xyz + clamp(blockInfo.w * blockSamplePosition, 0.5, blockInfo.w - 0.5)) / _lodTextureSize;
    //
    //return texture(_lodTexture, lodTexCoord).r;

    //vec4 blockInfo = texelFetch(_indexTexture, indexTexel, 0);
    //return performIntrablockSampling(blockInfo, blockSamplePosition);
};

/**
 * Performs the raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    float firstHitT = -1.0;

    // calculate ray parameters
    vec3 direction = exitPoint.rgb - entryPoint.rgb;
    float t = 0.0;
    float tend = length(direction);
    direction = normalize(direction);

    jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    while (t < tend) {
        // compute sample position
        vec3 samplePosition = entryPoint.rgb + t * direction;

        vec3 thisBlockLlf = floor((samplePosition * _indexTextureSize) / _nonNpotVolumeCompensationMultiplier);
        vec3 thisBlockUrb = thisBlockLlf + vec3(1);
        float thisBlockSize = texelFetch(_indexTexture, ivec3(thisBlockLlf), 0).w;

        // lookup intensity and TF
        vec8 intensityGradient = loookupIntensity(samplePosition);
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensityGradient.intensity.r);

        // perform compositing
        if (color.a > 0.0) {
#ifdef ENABLE_SHADING
            // compute gradient (needed for shading and normals)
            vec3 gradient = (intensityGradient.gradient.xyz - 0.5) * intensityGradient.gradient.w * -255;
            vec4 worldPos = _volumeTextureParams._textureToWorldMatrix * vec4(samplePosition, 1.0); // calling textureToWorld here crashes Intel HD driver and nVidia driver in debug mode, hence, let's calc it manually...
            color.rgb = calculatePhongShading(worldPos.xyz / worldPos.w, _lightSource, _cameraPosition, gradient, color.rgb);
#endif

            // accomodate for variable sampling rates
            color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * (16/thisBlockSize) * SAMPLING_BASE_INTERVAL_RCP);
            result.rgb = result.rgb + color.rgb * color.a  * (1.0 - result.a);
            result.a = result.a + (1.0 -result.a) * color.a;

            // save first hit ray parameter for depth value calculation
            if (firstHitT < 0.0 && result.a > 0.0) {
                firstHitT = t;
                out_FHP = vec4(samplePosition, 1.0);
                out_FHN = vec4(gradient, 1.0);
            }
        }

        // early ray termination
        if (result.a > 0.975) {
            result.a = 1.0;
            t = tend;
        }

        
        float rayBlockIntersection = IntersectBoxOnlyTFar(samplePosition, direction, _nonNpotVolumeCompensationMultiplier * thisBlockLlf/_indexTextureSize, _nonNpotVolumeCompensationMultiplier * thisBlockUrb/_indexTextureSize);
        jitterFloat(rayBlockIntersection, _samplingStepSize * _jitterStepSizeMultiplier);

        // advance to the next evaluation point along the ray
        t += min(_samplingStepSize * (16/thisBlockSize), _samplingStepSize + rayBlockIntersection);
    }

    // calculate depth value from ray parameter
    gl_FragDepth = 1.0;
    if (firstHitT >= 0.0) {
        float depthEntry = texture(_entryPointsDepth, texCoords).z;
        float depthExit = texture(_exitPointsDepth, texCoords).z;
        gl_FragDepth = calculateDepthValue(firstHitT/tend, depthEntry, depthExit);
    }
    return result;
}

/***
 * The main method.
 ***/
void main() {
    vec2 p = gl_FragCoord.xy * _viewportSizeRCP;
    vec3 frontPos = texture(_entryPoints, p).rgb;
    vec3 backPos = texture(_exitPoints, p).rgb;

    //determine whether the ray has to be casted
    if (frontPos == backPos) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        out_Color = performRaycasting(frontPos, backPos, p);
    }
}
