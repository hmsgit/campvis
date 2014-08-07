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

#include "tools/colorspace.frag"
#include "tools/gradient.frag"
#include "tools/raycasting.frag"
#include "tools/shading.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

#define NUM_PREDICATE_SETS 3

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

// volume
uniform sampler3D _volume;
uniform TextureParameters3D _volumeTextureParams;

uniform sampler3D _cm;
uniform TextureParameters3D _cmParams;
uniform sampler3D _tc;
uniform TextureParameters3D _tcParams;
uniform sampler3D _plaque;
uniform TextureParameters3D _plaqueParams;

uniform float _minDepth;
uniform float _maxDepth;

// Transfer function
uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;

// TODO: copy+paste from Voreen - eliminate or improve.
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

uniform vec2 _intensityRange = vec2(0.05, 1.0);
uniform float _intensityImportance = 1.0;

uniform vec2 _gradientMagnitudeRange = vec2(0.03, 1.0);
uniform float _gradientMagnitudeImportance = 2.0;

uniform int _boneBit = 3;
uniform float _boneImportance = 1.2;

uniform int _muscleBit = 1;
uniform float _muscleImportance = 1.75;

uniform int _contextBit = 2;
uniform float _contextImportance = 1.2;

uniform vec3 _scanningDirection = vec3(0.0, 1.0, 0.0);

float hadd(in vec3 v) {
    return v.x + v.y + v.z;
}

float hmax(in vec3 v) {
    return max(v.x, max(v.y, v.z));
}

/**
 * Performs the raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    float importance = 0.0;
    float firstHitT = -1.0;
    
    // calculate ray parameters
    vec3 direction = exitPoint.rgb - entryPoint.rgb;
    float t = 0.0;
    float tend = length(direction);
    direction = normalize(direction);

    float depthEntry = texture(_entryPointsDepth, texCoords).z;
    float depthExit = texture(_exitPointsDepth, texCoords).z;

    jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    while (t < tend) {
        // compute sample position
        vec3 samplePosition = entryPoint.rgb + t * direction;

        // lookup intensity and TF
        float intensity = texture(_volume, samplePosition).r;
        float D = smoothstep(_minDepth, _maxDepth, calculateDepthValue(t/tend, depthEntry, depthExit));
        vec3 gradient = computeGradient(_volume, _volumeTextureParams, samplePosition);

        // perform compositing
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

        if (color.a > 0.0) {
            float cm = texture(_cm, samplePosition).r;
            vec4 tc = texture(_tc, samplePosition);
            float plaque = texture(_plaque, samplePosition).r;
            vec3 worldPosition = textureToWorld(_volumeTextureParams, samplePosition).xyz;

            vec4 pr = performPredicateBasedShading(intensity, cm, tc, plaque);
            float imp = pr.a;

            if (imp > 0.0) {
//                if (pr.z > 0.0)
//                    color = clamp(color + pr.z, 0.0, 1.0);

    #ifdef ENABLE_SHADING
                // compute gradient (needed for shading and normals)
                color.rgb = calculatePhongShading(worldPosition, _lightSource, _cameraPosition, gradient, color.rgb, color.rgb, vec3(1.0, 1.0, 1.0));
    #endif

                // perform predicate-based shading
                color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
                
                vec3 hsl = rgb2hcy(color.rgb * color.a);
                hsl.xy += pr.xy;
                hsl.x = mod(hsl.x, 1.0);
                //hsl.z = cm / 2.0;
                color.rgb = hcy2rgb(hsl);

                // perform depth-based shading
                const float DBS = 0.01;
                //color.r += DBS * (0.5 - D);
                //color.b += DBS * (D - 0.5);
                                
                float vis = 1.0 - exp(importance - imp);
                float m = (imp <= importance || (1.0 - result.a) >= vis) ? 1.0 : (1.0 - vis) / result.a;

                vec3 cc = m * result.rgb + (1.0 - m * result.a) * color.rgb;
                float aa = m * result.a * (1.0 - color.a) + color.a;
                result.a = result.a * (1.0 - color.a) + color.a;
                result.rgb = (aa == 0.0) ? vec3(0.0) : (result.a * cc) / aa;
                importance = max(importance, log(color.a + (1.0 - color.a) * exp(importance - imp)) + imp);
            }
        }

        // save first hit ray parameter for depth value calculation
        if (firstHitT < 0.0 && result.a > 0.0) {
            firstHitT = t;
            out_FHP = vec4(samplePosition, 1.0);
            out_FHN = vec4(normalize(computeGradient(_volume, _volumeTextureParams, samplePosition)), 1.0);
        }

        // advance to the next evaluation point along the ray
        t += _samplingStepSize;
    }

    // calculate depth value from ray parameter
    gl_FragDepth = 1.0;
    if (firstHitT >= 0.0) {
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
