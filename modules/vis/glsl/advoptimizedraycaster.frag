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

// Input volume
uniform sampler3D _volume;
uniform TextureParameters3D _volumeTextureParams;

// Transfer function
uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

// Voxel Hierarchy Lookup volume
uniform usampler2D _voxelHierarchy;
uniform int _vhMaxMipMapLevel;

uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

/**
 * Performs the raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    vec3 direction = exitPoint - entryPoint;
    float len = length(direction);
    
    // Adjust direction a bit to prevent division by zero
    direction.x = (abs(direction.x) < 0.0000001) ? 0.0000001 : direction.x;
    direction.y = (abs(direction.y) < 0.0000001) ? 0.0000001 : direction.y;
    direction.z = (abs(direction.z) < 0.0000001) ? 0.0000001 : direction.z;

    OFFSET = (0.25 / (1 << _vhMaxMipMapLevel)); //< offset value used to avoid self-intersection or previous voxel intersection.

    jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    float firstHitT = -1.0f;

    float tNear = clipFirstHitpoint(_voxelHierarchy, _vhMaxMipMapLevel, entryPoint, direction, 0.0, 1.0);
    float tFar = 1.0 - clipFirstHitpoint(_voxelHierarchy, _vhMaxMipMapLevel, exitPoint, -direction, 0.0, 1.0);

    // compute sample position
    vec3 samplePosition = entryPoint.rgb + tNear * direction;

    while (tNear < tFar) {
        vec3 samplePosition = entryPoint.rgb + tNear * direction;

        // lookup intensity and TF
        float intensity = texture(_volume, samplePosition).r;
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

        // perform compositing
        if (color.a > 0.0) {
#ifdef ENABLE_SHADING
            // compute gradient (needed for shading and normals)
            vec3 gradient = computeGradient(_volume, _volumeTextureParams, samplePosition);
            vec4 worldPos = _volumeTextureParams._textureToWorldMatrix * vec4(samplePosition, 1.0); // calling textureToWorld here crashes Intel HD driver and nVidia driver in debug mode, hence, let's calc it manually...
            color.rgb = calculatePhongShading(worldPos.xyz / worldPos.w, _lightSource, _cameraPosition, gradient, color.rgb);
#endif

            // accomodate for variable sampling rates
            color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
            result.rgb = result.rgb + color.rgb * color.a  * (1.0 - result.a);
            result.a = result.a + (1.0 -result.a) * color.a;
        }

        // save first hit ray parameter for depth value calculation
        if (firstHitT < 0.0 && result.a > 0.0) {
            firstHitT = tNear;
            out_FHP = vec4(samplePosition, 1.0);
            out_FHN = vec4(normalize(computeGradient(_volume, _volumeTextureParams, samplePosition)), 1.0);
        }

        // early ray termination
        if (result.a > 0.975) {
            result.a = 1.0;
            tNear = tFar;
        
                
        }

        // advance to the next evaluation point along the ray
        tNear += _samplingStepSize / len;
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
