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

// DRR volume
uniform sampler3D _volume;
uniform TextureParameters3D _volumeTextureParams;

// Transfer function
uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;


// Illumination cache
uniform layout(r32f) image2D _icImageIn;
uniform layout(r32f) image2D _icImageOut;
uniform vec3 _icOrigin;
uniform vec3 _icNormal;
uniform vec3 _icRightVector;
uniform vec3 _icUpVector;
uniform float _shadowIntensity = 0.5;

// Voxel Hierarchy Lookup volume
uniform usampler2D _voxelHierarchy;
uniform int _vhMaxMipMapLevel;


uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

// projects a vector in world coordinates onto the IC
// returns world coordinates
ivec2 calcIcSamplePosition(vec3 worldPosition) {
    // project world position onto IC plane
    const vec3 diag = worldPosition - _icOrigin;
    const float distance = abs(dot(diag, _icNormal));
    const vec3 worldProjected = diag - (-distance * _icNormal);

    // transforms world coordinates (have to be lying on the IC plane) to IC pixel space
    return ivec2(round(dot(worldProjected, _icRightVector)), round(dot(worldProjected, _icUpVector)));
}

// the composite function can be used for additional shadow ray integration
// from the current sample to the position of the IC.
// However, it's currently not used for performance reasons
void composite(vec3 startPosition, vec3 endPosition, inout float opacity) {
    vec3 direction = endPosition - startPosition;
    float t = _samplingStepSize;
    jitterFloat(t, _samplingStepSize); // jitter startpoint to avoid ringing artifacts (not really effective...)

    float tend = min(length(direction), 4*_samplingStepSize);
    direction = normalize(direction);

    while (t < tend) {
        // lookup intensity and TF
        vec3 samplePosition = startPosition.xyz + t * direction;
        float intensity = texture(_volume, samplePosition).r;
        float tfOpacity = lookupTF(_transferFunction, _transferFunctionParams, intensity).a;
        opacity = opacity + (1.0 - opacity) * tfOpacity;

        t += _samplingStepSize;
    }
}


/**
 * Performs the raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    float firstHitT = -1.0;

    // calculate ray parameters
    vec3 direction = exitPoint.rgb - entryPoint.rgb;
    float len = length(direction);
    
    // Adjust direction a bit to prevent division by zero
    direction.x = (abs(direction.x) < 0.0001) ? 0.0001 : direction.x;
    direction.y = (abs(direction.y) < 0.0001) ? 0.0001 : direction.y;
    direction.z = (abs(direction.z) < 0.0001) ? 0.0001 : direction.z;    
    OFFSET = (0.25 / (1 << _vhMaxMipMapLevel)); //< offset value used to avoid self-intersection or previous voxel intersection.

    float t = clipFirstHitpoint(_voxelHierarchy, _vhMaxMipMapLevel, entryPoint, direction, 0.0, 1.0);
    float tend = 1.0 - clipFirstHitpoint(_voxelHierarchy, _vhMaxMipMapLevel, exitPoint, -direction, 0.0, 1.0);
    jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    ivec2 icPositionPrev = calcIcSamplePosition(textureToWorld(_volumeTextureParams, entryPoint.rgb + t * direction));
    float icIn = imageLoad(_icImageIn, icPositionPrev).r;
    float icOut = (0.0);
    bool toBeSaved = false;

    while (t < tend) {
        // compute sample position
        vec3 samplePosition = entryPoint.rgb + t * direction;
        vec3 worldPos = textureToWorld(_volumeTextureParams, samplePosition);
        ivec2 icPosition = calcIcSamplePosition(worldPos);

        // optimization: Only store/load when the icPosition has changed
        // otherwise we can reuse the variables from the previous sample
        if (icPositionPrev != icPosition) {
            // write illumination information
            if (toBeSaved)
                imageStore(_icImageOut, icPositionPrev, vec4(icOut));
            toBeSaved = false;
            
            // load illumination information
            icIn = imageLoad(_icImageIn, icPosition).r;

            // perform a compositing from samplePosition to the samplePosition of the IC
            // Currently disabled since it leads to ringing artifacts...
            //if (icIn.xyz != vec3(0.0))
            //    composite(samplePosition, icIn.xyz, icIn.a);
        }

        // lookup intensity and TF
        float intensity = texture(_volume, samplePosition).r;
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

        // perform compositing
        if (color.a > 0.0) {
            // compute gradient (needed for shading and normals)
            vec3 gradient = computeGradient(_volume, _volumeTextureParams, samplePosition);            

            // accomodate for variable sampling rates
            color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);

            // perform global illumination
            // back-to-front compositing from light-direction 
            // (for now, we ignore the color contribution and store the world position instead)
            // icOut.rgb = ((1.0 - color.a) * icIn.rgb) + (color.a * color.rgb);
            //icOut.xyz = samplePosition;
            icOut   = ((1.0 - color.a) * icIn) + color.a;
            toBeSaved = true;

            // apply shadowing
            const vec3 ambientColorOverride = _lightSource._ambientColor * (1.0 - icIn * _shadowIntensity);
            color.rgb = calculatePhongShading(worldPos, ambientColorOverride, _lightSource, _cameraPosition, gradient, color.rgb);

            // front-to-back compositing along view direction
            result.rgb = result.rgb + color.rgb * color.a  * (1.0 - result.a);
            result.a = result.a + (1.0 -result.a) * color.a;

            icPositionPrev = icPosition;
        }

        // save first hit ray parameter for depth value calculation
        if (firstHitT < 0.0 && result.a > 0.0) {
            firstHitT = t;
            out_FHP = vec4(samplePosition, 1.0);
            out_FHN = vec4(icPosition, 0.0, 0.0);// vec4(normalize(computeGradient(_volume, _volumeTextureParams, samplePosition)), 1.0);
        }

        // early ray termination (disabled!)
        //if (result.a > 0.975) {
        //    result.a = 1.0;
        //    t = tend;
        //}

        // advance to the next evaluation point along the ray
        t += _samplingStepSize / len;
    }

    if (toBeSaved)
        imageStore(_icImageOut, icPositionPrev, vec4(icOut));

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
 