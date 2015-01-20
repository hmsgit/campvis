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

uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

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
        vec3 worldPosition = entryPoint.rgb + t * direction;


        // FIRST volume
        vec3 samplePosition1 = worldToTexture(_volumeParams1, worldPosition).xyz;
        if (all(greaterThanEqual(samplePosition1, vec3(0.0, 0.0, 0.0))) && all(lessThanEqual(samplePosition1, vec3(1.0, 1.0, 1.0)))) {
            // lookup intensity and TF
            float intensity = texture(_volume1, samplePosition1).r;
            vec4 color = lookupTF(_transferFunction1, _transferFunctionParams1, intensity);

            // perform compositing
            if (color.a > 0.0) {
                // compute gradient (needed for shading and normals)
                vec3 gradient = computeGradient(_volume1, _volumeParams1, samplePosition1);
                color.rgb = calculatePhongShading(worldPosition, _lightSource, _cameraPosition, gradient, color.rgb, color.rgb, vec3(1.0, 1.0, 1.0));

                // accomodate for variable sampling rates
                color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
                result.rgb = result.rgb + color.rgb * color.a  * (1.0 - result.a);
                result.a = result.a + (1.0 -result.a) * color.a;
            }
        }


        // SECOND volume
        vec3 samplePosition2 = worldToTexture(_volumeParams2, worldPosition).xyz;
        if (all(greaterThanEqual(samplePosition2, vec3(0.0, 0.0, 0.0))) && all(lessThanEqual(samplePosition2, vec3(1.0, 1.0, 1.0)))) {
            // lookup intensity and TF
            float intensity = texture(_volume2, samplePosition2).r;
            vec4 color = lookupTF(_transferFunction2, _transferFunctionParams2, intensity);

            // perform compositing
            if (color.a > 0.0) {
                // compute gradient (needed for shading and normals)
                vec3 gradient = computeGradient(_volume2, _volumeParams2, samplePosition2);
                color.rgb = calculatePhongShading(worldPosition, _lightSource, _cameraPosition, gradient, color.rgb, color.rgb, vec3(1.0, 1.0, 1.0));

                // accomodate for variable sampling rates
                color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
                result.rgb = result.rgb + color.rgb * color.a  * (1.0 - result.a);
                result.a = result.a + (1.0 -result.a) * color.a;
            }
        }


        // THIRD volume
        vec3 samplePosition3 = worldToTexture(_volumeParams3, worldPosition).xyz;
        if (all(greaterThanEqual(samplePosition3, vec3(0.0, 0.0, 0.0))) && all(lessThanEqual(samplePosition3, vec3(1.0, 1.0, 1.0)))) {
            // lookup intensity and TF
            float intensity = texture(_volume3, samplePosition3).r;
            vec4 color = lookupTF(_transferFunction3, _transferFunctionParams3, intensity);

            // perform compositing
            if (color.a > 0.0) {
                // compute gradient (needed for shading and normals)
                vec3 gradient = computeGradient(_volume3, _volumeParams3, samplePosition3);
                color.rgb = calculatePhongShading(worldPosition, _lightSource, _cameraPosition, gradient, color.rgb, color.rgb, vec3(1.0, 1.0, 1.0));

                // accomodate for variable sampling rates
                color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
                result.rgb = result.rgb + color.rgb * color.a  * (1.0 - result.a);
                result.a = result.a + (1.0 -result.a) * color.a;
            }
        }



        // save first hit ray parameter for depth value calculation
        if (firstHitT < 0.0 && result.a > 0.0) {
            firstHitT = t;
            out_FHP = vec4(worldPosition, 1.0);
            out_FHN = vec4(normalize(computeGradient(_volume1, _volumeParams1, worldPosition)), 1.0);
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
