// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#version 330

out vec4 out_Color;         ///< outgoing fragment color

#include "tools/raycasting.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform vec2 _viewportSizeRCP;
uniform bool _jitterEntryPoints;
uniform float _jitterStepSizeMultiplier;

uniform Texture2D _entryPoints;          // ray entry points
uniform Texture2D _exitPoints;           // ray exit points
uniform Texture3D _volume;            // texture lookup parameters for volume_

uniform TransferFunction1D _transferFunction;

uniform float _samplingStepSize;
uniform float _shift;
uniform float _scale;

// TODO: copy+paste from Voreen - eliminate or improve.
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

    if (_jitterEntryPoints)
        jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    while (t < tend) {
        // compute sample position
        vec3 samplePosition = entryPoint.rgb + t * direction;

        // lookup intensity and TF
        float intensity = getElement3DNormalized(_volume, samplePosition).a;
        vec4 color = lookupTF(_transferFunction, intensity);

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
    vec3 frontPos = getElement2DNormalized(_entryPoints, p).rgb;
    vec3 backPos = getElement2DNormalized(_exitPoints, p).rgb;

    //determine whether the ray has to be casted
    if (frontPos == backPos) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        out_Color = raycastDRR(frontPos, backPos);
    }
}
