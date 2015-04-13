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

out vec4 out_Color;         ///< outgoing fragment color

#include "tools/raycasting.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform vec2 _viewportSizeRCP;
uniform float _jitterStepSizeMultiplier;

// ray entry points
uniform sampler2D _entryPoints;
uniform TextureParameters2D _entryParams;

// ray exit points
uniform sampler2D _exitPoints;
uniform TextureParameters2D _exitParams;

// DRR volume
uniform sampler3D _volume;
uniform TextureParameters3D _volumeTextureParams;

// Transfer function
uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform float _samplingStepSize;
uniform float _shift;
uniform float _scale;

const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

/**
 * Computes the DRR by simple raycasting and returns the final fragment color.
 */
vec4 raycastDRR(in vec3 entryPoint, in vec3 exitPoint) {
    vec4 result = vec4(0.0);

    // calculate ray parameters
    vec3 direction = exitPoint.rgb - entryPoint.rgb;
    float t = 0.0;
    float tend = length(direction);
    direction = normalize(direction);

    jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    while (t < tend) {
        // compute sample position
        vec3 samplePosition = entryPoint.rgb + t * direction;

        // lookup intensity and TF
        float intensity = texture(_volume, samplePosition).r;
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

#ifdef DEPTH_MAPPING
        // use Bernstein Polynomials for depth-color mapping
        float depth = t / tend;
            
        float b02 = (1-depth) * (1-depth);
        float b12 = 2 * depth * (1-depth);
        float b22 = depth * depth;
            
        color = vec4(0.75*b02 + 0.25*b12, b12, 0.25*b12 + 0.75*b22, 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP));
/*            float t_sq = depth * depth;
        float inv_t_sq = (1-depth) * (1-depth);
        float b04 = inv_t_sq * inv_t_sq;
        float b24 = 6 * t_sq * inv_t_sq;
        float b44 = t_sq * t_sq;
        color = vec4(0.5*b04 + 0.5*b24, b24, 0.5*b24 + 0.5*b44, 1.0 - pow(1.0 - color.a, samplingStepSize_ * SAMPLING_BASE_INTERVAL_RCP));*/

        result.rgb = result.rgb + (1.0 - result.a) * color.a * color.rgb;
        result.a = result.a + color.a * _scale;
#else
        // perform compositing (account for differen step size / sampling rate)
        color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
        result.a = result.a + color.a * _scale;
#endif       

        t += _samplingStepSize;
    }

#ifdef DEPTH_MAPPING
    result.rgb = result.rgb * vec3(exp(-result.a + _shift));
    //result.rgb = exp(-result.rgb * result.a + shift_);
#else
#ifdef DRR_INVERT
    // apply e-function, as we want the "negative" X-Ray image, we invert the intensity result
    result.rgb = 1 - vec3(exp(-result.a + _shift));
#else
    // apply e-function, the normal way
    result.rgb = vec3(exp(-result.a + _shift));
#endif
#endif
    result.a = 1.0;
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
        out_Color = raycastDRR(frontPos, backPos);
    }
}
