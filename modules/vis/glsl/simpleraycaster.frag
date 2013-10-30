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

#ifdef ENABLE_ADAPTIVE_STEPSIZE
bool _inVoid = false;
#endif

#ifdef ENABLE_SHADOWING
uniform float _shadowIntensity;
#endif

// TODO: copy+paste from Voreen - eliminate or improve.
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

ivec3 voxelToBrick(in vec3 voxel) {
    return ivec3(floor(voxel / _bbvBrickSize));
}

int brickToIndex(in ivec3 brick) {
    return int(brick.x + (_bbvTextureParams._size.x * brick.y) + (_bbvTextureParams._size.x * _bbvTextureParams._size.y * brick.z));
}

// samplePosition is in texture coordiantes [0, 1]
bool lookupInBbv(in vec3 samplePosition) {
    ivec3 brick = voxelToBrick(samplePosition * _volumeTextureParams._size);

    ivec3 byte = brick;
    byte.x /= 8;
    uint bit = uint(brick.x % 8);
    uint texel = texelFetch(_bbvTexture, byte, 0).r;

    return (texel & (1U << bit)) != 0U;
}

/**
 * Performs the raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    float firstHitT = -1.0;
#ifdef ENABLE_ADAPTIVE_STEPSIZE
    float samplingRateCompensationMultiplier = 1.0;
#endif

    // calculate ray parameters
    vec3 direction = exitPoint.rgb - entryPoint.rgb;
    float t = 0.0;
    float tend = length(direction);
    direction = normalize(direction);

    jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    while (t < tend) {
        // compute sample position
        vec3 samplePosition = entryPoint.rgb + t * direction;

        if (_hasBbv) {
            if (lookupInBbv(samplePosition)) {
        // advance to the next evaluation point along the ray
#ifdef ENABLE_ADAPTIVE_STEPSIZE
            samplingRateCompensationMultiplier = 1.0;
            t += _samplingStepSize;
#else
            t += _samplingStepSize;
#endif
            continue;
            }
        }

        // lookup intensity and TF
        float intensity = getElement3DNormalized(_volume, _volumeTextureParams, samplePosition).a;
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

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
                    float newIntensity = getElement3DNormalized(_volume, _volumeTextureParams, newSamplePosition).a;
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

#ifdef ENABLE_SHADOWING
        // simple and expensive implementation of hard shadows
        if (color.a > 0.1) {
            // compute direction from sample to light
            vec3 L = normalize(_lightSource._position - textureToWorld(_volumeTextureParams, samplePosition).xyz) * _samplingStepSize;

            bool finished = false;
            vec3 position = samplePosition + L;
            float shadowFactor = 0.0;

            // traverse ray from sample to light
            while (! finished) {
                // grab intensity and TF opacity
                intensity = getElement3DNormalized(_volume, _volumeTextureParams, position).a;
                shadowFactor += lookupTF(_transferFunction, _transferFunctionParams, intensity).a;

                position += L;
                finished = (shadowFactor > 0.95)
                         || any(lessThan(position, vec3(0.0, 0.0, 0.0)))
                         || any(greaterThan(position, vec3(1.0, 1.0, 1.0)));
            }
            // apply shadow to color
            color.rgb *= (1.0 - shadowFactor * _shadowIntensity);
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
#ifdef ENABLE_ADAPTIVE_STEPSIZE
            color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * samplingRateCompensationMultiplier * SAMPLING_BASE_INTERVAL_RCP);
#else
            color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
#endif
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
#ifdef ENABLE_ADAPTIVE_STEPSIZE
        samplingRateCompensationMultiplier = (_inVoid ? 1.0 : 0.25);
        t += _samplingStepSize * (_inVoid ? 1.0 : 0.125);
        
#else
        t += _samplingStepSize;
#endif
    }

    // calculate depth value from ray parameter
    gl_FragDepth = 1.0;
    if (firstHitT >= 0.0) {
        float depthEntry = getElement2DNormalized(_entryPointsDepth, _entryParams, texCoords).z;
        float depthExit = getElement2DNormalized(_exitPointsDepth, _exitParams, texCoords).z;
        gl_FragDepth = calculateDepthValue(firstHitT/tend, depthEntry, depthExit);
    }
    return result;
}

/***
 * The main method.
 ***/
void main() {
    vec2 p = gl_FragCoord.xy * _viewportSizeRCP;
    vec3 frontPos = getElement2DNormalized(_entryPoints, _entryParams, p).rgb;
    vec3 backPos = getElement2DNormalized(_exitPoints, _exitParams, p).rgb;

    //determine whether the ray has to be casted
    if (frontPos == backPos) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        out_Color = performRaycasting(frontPos, backPos, p);
    }
}
