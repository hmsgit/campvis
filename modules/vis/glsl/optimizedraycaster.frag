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

layout(location = 0) out vec4 out_Color;     ///< outgoing fragment color
layout(location = 1) out vec4 out_FHP;       ///< outgoing fragment first hitpoint
layout(location = 2) out vec4 out_FHN;       ///< outgoing fragment first hit normal

#include "tools/gradient.frag"
#include "tools/raycasting.frag"
#include "tools/shading.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

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

// DRR volume
uniform sampler3D _volume;
uniform TextureParameters3D _volumeTextureParams;

// Transfer function
uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;


// BBV Lookup volume
uniform usampler3D _bbvTexture;
uniform TextureParameters3D _bbvTextureParams;
uniform int _bbvBrickSize;
uniform bool _hasBbv;


uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;

#ifdef INTERSECTION_REFINEMENT
bool _inVoid = false;
#endif

#ifdef ENABLE_SHADOWING
uniform float _shadowIntensity;
#endif

const float positiveInfinity = 1.0 / 0.0;

// TODO: copy+paste from Voreen - eliminate or improve.
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

ivec3 voxelToBrick(in vec3 voxel) {
    return ivec3(floor(voxel / _bbvBrickSize));
}

// samplePosition is in texture coordiantes [0, 1]
bool lookupInBbv(in vec3 samplePosition) {
    ivec3 byte = voxelToBrick(samplePosition * _volumeTextureParams._size);
    uint bit = uint(byte.x % 8);
    byte.x /= 8;

    uint texel = texelFetch(_bbvTexture, byte, 0).r;

    return (texel & (1U << bit)) != 0U;
}

float rayBoxIntersection(in vec3 rayOrigin, in vec3 rayDirection, in vec3 boxLlf, in vec3 boxUrb, in float t) {
    vec3 tMin = (boxLlf - rayOrigin) / rayDirection;
    vec3 tMax = (boxUrb - rayOrigin) / rayDirection;

    // TODO: these many ifs are expensive - the lessThan bvec solution below should be faster but does not work for some reason...
    if (tMin.x < t) tMin.x = positiveInfinity;
    if (tMin.y < t) tMin.y = positiveInfinity;
    if (tMin.z < t) tMin.z = positiveInfinity;

    if (tMax.x < t) tMax.x = positiveInfinity;
    if (tMax.y < t) tMax.y = positiveInfinity;
    if (tMax.z < t) tMax.z = positiveInfinity;

    //tMin += vec3(lessThan(tMin, vec3(t, t, t))) * positiveInfinity;
    //tMax += vec3(lessThan(tMax, vec3(t, t, t))) * positiveInfinity;

    return min(min(tMin.x, min(tMin.y, tMin.z))   ,    min(tMax.x, min(tMax.y, tMax.z)));
}

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

        // check whether we have a lookup volume for empty space skipping
        if (_hasBbv) {
            if (!lookupInBbv(samplePosition)) {
                // advance the ray to the intersection point with the current brick
                vec3 brickVoxel = floor((samplePosition * _volumeTextureParams._size) / _bbvBrickSize) * _bbvBrickSize;
                vec3 boxLlf = brickVoxel * _volumeTextureParams._sizeRCP;
                vec3 boxUrb = boxLlf + (_volumeTextureParams._sizeRCP * _bbvBrickSize);

                t = rayBoxIntersection(entryPoint, direction, boxLlf, boxUrb, t) + _samplingStepSize;
                continue;
            }
        }

        // lookup intensity and TF
        float intensity = texture(_volume, samplePosition).r;
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

#ifdef INTERSECTION_REFINEMENT
        if (color.a <= 0.0) {
            // we're within void, make the steps bigger
            _inVoid = true;
        }
        else {
            if (_inVoid) {
                float formerT = t - _samplingStepSize;

                // we just left the void, perform intersection refinement
                for (float stepPower = 0.5; stepPower > 0.1; stepPower /= 2.0) {
                    // compute refined sample position
                    float newT = formerT + _samplingStepSize * stepPower;
                    vec3 newSamplePosition = entryPoint.rgb + newT * direction;

                    // lookup refined intensity + TF
                    float newIntensity = texture(_volume, newSamplePosition).r;
                    vec4 newColor = lookupTF(_transferFunction, _transferFunctionParams, newIntensity);

                    if (newColor.a <= 0.0) {
                        // we're back in the void - look on the right-hand side
                        formerT = newT;
                    }
                    else {
                        // we're still in the matter - look on the left-hand side
                        samplePosition = newSamplePosition;
                        color = newColor;
                        t -= _samplingStepSize * stepPower;
                    }
                }
                _inVoid = false;
            }
        }
#endif

        // perform compositing
        if (color.a > 0.0) {
#ifdef ENABLE_SHADING
            // compute gradient (needed for shading and normals)
            vec3 gradient = computeGradient(_volume, _volumeTextureParams, samplePosition);
            color.rgb = calculatePhongShading(textureToWorld(_volumeTextureParams, samplePosition).xyz, _lightSource, _cameraPosition, gradient, color.rgb, color.rgb, vec3(1.0, 1.0, 1.0));
#endif

            // accomodate for variable sampling rates
            color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
            result.rgb = mix(color.rgb, result.rgb, result.a);
            result.a = result.a + (1.0 -result.a) * color.a;
        }

        // save first hit ray parameter for depth value calculation
        if (firstHitT < 0.0 && result.a > 0.0) {
            firstHitT = t;
            out_FHP = vec4(samplePosition, 1.0);
            out_FHN = vec4(normalize(computeGradient(_volume, _volumeTextureParams, samplePosition)), 1.0);
        }

        // early ray termination
        if (result.a > 0.975) {
            result.a = 1.0;
            t = tend;
        }

        // advance to the next evaluation point along the ray
        t += _samplingStepSize;
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
