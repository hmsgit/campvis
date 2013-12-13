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

#include "tools/gradient.frag"
#include "tools/raycasting.frag"
#include "tools/shading.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform vec2 _viewportSizeRCP;
uniform bool _jitterEntryPoints;
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


uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;

#ifdef ENABLE_ADAPTIVE_STEPSIZE
bool _inVoid = false;
#endif

// TODO: copy+paste from Voreen - eliminate or improve.
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

    if (_jitterEntryPoints)
        jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    while (t < tend) {
        // compute sample position
        vec3 samplePosition = entryPoint.rgb + t * direction;

        // lookup intensity and TF
        vec4 strain = texture(_volume, samplePosition);
        vec4 color = (_volumeTextureParams._numChannels == 4) ? strain : vec4(strain.xyz, 0.0);
        color.a = clamp(length(strain.xyz)/1.0, 0.0, 1.0);

#ifdef ENABLE_ADAPTIVE_STEPSIZE
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
                    vec4 newStrain = texture(_volume, newSamplePosition);
                    vec4 newColor = (_volumeTextureParams._numChannels == 4) ? newStrain : vec4(newStrain.xyz, 0.0);
                    newColor.a = clamp(length(newColor.xyz)/1.0, 0.0, 1.0);

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
        }

        // early ray termination
        if (result.a > 0.975) {
            result.a = 1.0;
            t = tend;
        }

#ifdef ENABLE_ADAPTIVE_STEPSIZE
        t += _samplingStepSize * (_inVoid ? 1.0 : 0.125);
#else
        t += _samplingStepSize;
#endif
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
