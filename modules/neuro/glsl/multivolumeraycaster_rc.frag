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

// Input volume2
uniform sampler3D _volume1;
uniform sampler3D _volume2;
uniform sampler3D _volume3;
uniform TextureParameters3D _volumeParams1;
uniform TextureParameters3D _volumeParams2;
uniform TextureParameters3D _volumeParams3;

// Transfer function
uniform sampler1D _transferFunction1;
uniform sampler1D _transferFunction2;
uniform sampler1D _transferFunction3;
uniform TFParameters1D _transferFunctionParams1;
uniform TFParameters1D _transferFunctionParams2;
uniform TFParameters1D _transferFunctionParams3;

// Voxel Hierarchy Lookup volumes
uniform usampler2D _voxelHierarchy1;
uniform usampler2D _voxelHierarchy2;
uniform usampler2D _voxelHierarchy3;
uniform int _vhMaxMipMapLevel1;
uniform int _vhMaxMipMapLevel2;
uniform int _vhMaxMipMapLevel3;

uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

vec2 clipVolume(usampler2D vhTexture, int vhMaxMipmapLevel, TextureParameters3D volumeParams, in vec3 entryPoint, in vec3 exitPoint) {
    vec3 startPosTex = worldToTexture(volumeParams, entryPoint).xyz;
    vec3 endPosTex = worldToTexture(volumeParams, exitPoint).xyz;
    vec3 directionTex = endPosTex - startPosTex;

    float tNear = clipFirstHitpoint(vhTexture, vhMaxMipmapLevel, startPosTex, directionTex, 0.0, 1.0);
    float tFar = 1.0 - clipFirstHitpoint(vhTexture, vhMaxMipmapLevel, endPosTex, -directionTex, 0.0, 1.0);

    return vec2(tNear, tFar);
}

#define RAYCASTING_STEP(worldPosition, CLIP, volume, volumeParams, tf, tfParams, result, firstHitT, tNear) \
    { \
    if (tNear >= CLIP.x && tNear <= CLIP.y) { \
        vec3 samplePosition = worldToTexture(volumeParams, worldPosition).xyz; \
        if (all(greaterThanEqual(samplePosition, vec3(0.0, 0.0, 0.0))) && all(lessThanEqual(samplePosition, vec3(1.0, 1.0, 1.0)))) { \
            // lookup intensity and TF  \
            float intensity = texture(volume, samplePosition).r; \
            vec4 color = lookupTF(tf, tfParams, intensity); \
            \
            // perform compositing \
            if (color.a > 0.0) { \
                // compute gradient (needed for shading and normals) \
                vec3 gradient = computeGradient(volume, volumeParams, samplePosition); \
                color.rgb = calculatePhongShading(worldPosition, _lightSource, _cameraPosition, gradient, color.rgb); \
                \
                // accomodate for variable sampling rates \
                color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP); \
                result.rgb = result.rgb + color.rgb * color.a  * (1.0 - result.a); \
                result.a = result.a + (1.0 -result.a) * color.a; \
                \
                // save first hit ray parameter for depth value calculation \
                if (firstHitT < 0.0 && result.a > 0.01) { \
                    firstHitT = tNear; \
                    out_FHP = vec4(worldPosition, 1.0); \
                    out_FHN = vec4(gradient, 1.0); \
                } \
            } \
        } \
    } \
    }

/**
 * Performs the raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    float firstHitT = -1.0;

    // calculate ray parameters
    vec3 direction = exitPoint.rgb - entryPoint.rgb;

    OFFSET = (0.25 / (1 << _vhMaxMipMapLevel1)); //< offset value used to avoid self-intersection or previous voxel intersection.

    vec2 clip1 = clipVolume(_voxelHierarchy1, _vhMaxMipMapLevel1, _volumeParams1, entryPoint, exitPoint);
    vec2 clip2 = clipVolume(_voxelHierarchy2, _vhMaxMipMapLevel2, _volumeParams2, entryPoint, exitPoint);
    vec2 clip3 = clipVolume(_voxelHierarchy3, _vhMaxMipMapLevel3, _volumeParams3, entryPoint, exitPoint);
    float tNear = min(clip1.x, min(clip2.x, clip3.x));
    float tFar = max(clip1.y, max(clip2.y, clip3.y));

    jitterFloat(tNear, -_samplingStepSize * _jitterStepSizeMultiplier);

    while (tNear < tFar) {
        // compute sample position
        vec3 worldPosition = entryPoint.rgb + tNear * direction;
    
    
        // FIRST volume
        RAYCASTING_STEP(worldPosition, clip1, _volume1, _volumeParams1, _transferFunction1, _transferFunctionParams1, result, firstHitT, tNear);

        // SECOND volume
        RAYCASTING_STEP(worldPosition, clip2, _volume2, _volumeParams2, _transferFunction2, _transferFunctionParams2, result, firstHitT, tNear);

        // THIRD volume
        RAYCASTING_STEP(worldPosition, clip3, _volume3, _volumeParams3, _transferFunction3, _transferFunctionParams3, result, firstHitT, tNear);
        
        // early ray termination
        if (result.a > 0.975) {
            result.a = 1.0;
            tNear = tFar;
        }
    
        // advance to the next evaluation point along the ray
        tNear += _samplingStepSize;
    }

    // calculate depth value from ray parameter
    gl_FragDepth = 1.0;
    if (firstHitT >= 0.0) {
        float depthEntry = texture(_entryPointsDepth, texCoords).z;
        float depthExit = texture(_exitPointsDepth, texCoords).z;
        gl_FragDepth = calculateDepthValue(firstHitT, depthEntry, depthExit);
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
