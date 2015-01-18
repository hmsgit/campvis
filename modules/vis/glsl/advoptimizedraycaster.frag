// =============-===================================================================================
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
layout(location = 3) out vec4 out_count;       ///< outgoing fragment first hit normal

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

// XOR Bitmask texture
uniform usampler2D _xorBitmask;

// BBV Lookup volume
uniform usampler2D _vvTexture;
uniform int _vvVoxelSize;
uniform int _vvVoxelDepth;
uniform int _vvMaxMipMapLevel;
uniform ivec3 _vvSize;

uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;

#ifdef INTERSECTION_REFINEMENT
bool _inVoid = false;
#endif

#ifdef ENABLE_SHADOWING
uniform float _shadowIntensity;
#endif

// TODO: copy+paste from Voreen - eliminate or improve.
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

float originalTFar = -1.0;
const int MAXSTEPS = 50;
float OFFSET = 0.001;


// a minimal version of the method above
// (we assume: ray always hits the box)
float IntersectBoxOnlyTFar(in vec3 origin, in vec3 dir, in vec3 box_min, in vec3 box_max)
{
    vec3 tmin = (box_min - origin) / dir; 
    vec3 tmax = (box_max - origin) / dir; 

    vec3 real_max = max(tmin,tmax);

    // the minimal maximum is tFar
    // clamp to 1.0
    return min(1.0, min( min(real_max.x, real_max.y), real_max.z)); 
}

bool intersectBits(in uvec4 bitRay, in ivec2 texel, in int level, out uvec4 intersectionBitmask)
{
    //texel = clamp(texel, ivec2(0), ivec2(_vvSize / pow(2.0, level)) - 2);
    // Fetch bitmask from hierarchy and compute intersection via bitwise AND
    intersectionBitmask = (bitRay & texelFetch(_vvTexture, texel, level));
    return (intersectionBitmask != uvec4(0));
}

bool intersectHierarchy2(in vec3 origin, in vec3 direction, in int level, in vec3 posTNear, out float tFar, out uvec4 intersectionBitmask) {
    // Calculate pixel coordinates ([0,width]x[0,height])
    // of the current position along the ray
    float res = float(1 << (_vvMaxMipMapLevel - level));
    ivec2 pixelCoord = ivec2(posTNear.xy * res);

    // Voxel width and height in the unit cube
    vec2 voxelWH = vec2(1.0) / res;

    // Compute voxel stack (AABB) in the unit cube
    // belonging to this pixel position
    vec2 box_min = pixelCoord * voxelWH; // (left, bottom)

    // Compute intersection with the bounding box
    // It is always assumed that an intersection occurs and
    // that the position of posTNear remains the same
    tFar = IntersectBoxOnlyTFar(origin, direction,
        vec3(box_min, 0.0),
        vec3(box_min + voxelWH, 1.0));

    // Now test if some of the bits intersect
    float zFar = (tFar * direction.z) + origin.z ;

    // Fetch bitmask for ray and intersect with current pixel
    return intersectBits(
        texture(_xorBitmask, vec2(min(posTNear.z, zFar), max(posTNear.z, zFar))),
        pixelCoord, 
        level, 
        intersectionBitmask);
}

float lll;

float clipFirstHitpoint(in vec3 origin, in vec3 direction, in float tNear, in float tFar) {
    // Compute the exit position of the ray with the scene’s BB
    // tFar = rayBoxIntersection(origin, direction, vec3(0.0), vec3(1.0), tNear);
    // if (tFar > originalTFar) {
    //     vec3 foo = origin + tFar * direction;
    //     out_FHN = vec4(foo, 1.0);
    //     tFar = originalTFar;
    // }

    // Set current position along the ray to the ray’s origin
    vec3 posTNear = origin;
    bool intersectionFound = false;
    uvec4 intersectionBitmask = uvec4(0);

    // It’s faster to not start at the coarsest level
    int level = _vvMaxMipMapLevel / 2;
    for (int i = 0; (i < MAXSTEPS) && (tNear < tFar) && (!intersectionFound); i++) {
        float newTFar = 1.0;
        if (intersectHierarchy2(origin, direction, level, posTNear, newTFar, intersectionBitmask)) {
            // If we are at mipmap level 0 and an intersection occurred,
            // we have found an intersection of the ray with the volume
            intersectionFound = (level == 0);

            // Otherwise we have to move down one level and
            // start testing from there
            --level;
        }
        else {
            // If no intersection occurs, we have to advance the
            // position on the ray to test the next element of the hierachy.
            // Furthermore, add a small offset computed beforehand to
            // handle floating point inaccuracy.
            tNear = newTFar + OFFSET;
            posTNear = origin + tNear * direction;

            // Move one level up
            ++level;
        }
    }

    lll = float(level);
    return tNear;
}

/**
 * Performs the raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    vec3 direction = exitPoint - entryPoint;
    
    // Adjust direction a bit to prevent division by zero
    direction.x = (abs(direction.x) < 0.0000001) ? 0.0000001 : direction.x;
    direction.y = (abs(direction.y) < 0.0000001) ? 0.0000001 : direction.y;
    direction.z = (abs(direction.z) < 0.0000001) ? 0.0000001 : direction.z;

    float tNear = 0.0;
    float tFar = 1.0;
    OFFSET = (0.25 / (1 << _vvMaxMipMapLevel)) / tFar; //< offset value used to avoid self-intersection or previous voxel intersection.


    //jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    float firstHitT = -1.0f;

    tNear = clipFirstHitpoint(entryPoint, direction, 0.0, 1.0);
    tFar -= clipFirstHitpoint(exitPoint, -direction, 0.0, 1.0);

    originalTFar = tFar;
    //tNear *= length(direction);
    //tFar *= length(direction);
    //direction = normalize(direction);
 
    // compute sample position
    vec3 samplePosition = entryPoint.rgb + tNear * direction;

    out_FHP = vec4(samplePosition, 1.0);
    out_FHN = vec4(entryPoint.rgb + tFar * direction, 1.0);

    out_count = vec4(tNear, tFar, lll, 0.0);
    while (tNear < tFar) {
        vec3 samplePosition = entryPoint.rgb + tNear * direction;

        // lookup intensity and TF
        float intensity = texture(_volume, samplePosition).r;
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

        // perform compositing
        if (color.a > 0.0) {
#ifdef ENABLE_SHADING
            // compute gradient (needed for shading and normals)
            //vec3 gradient = computeGradient(_volume, _volumeTextureParams, samplePosition);
            //vec4 worldPos = _volumeTextureParams._textureToWorldMatrix * vec4(samplePosition, 1.0); // calling textureToWorld here crashes Intel HD driver and nVidia driver in debug mode, hence, let's calc it manually...
            //color.rgb = calculatePhongShading(worldPos.xyz / worldPos.w, _lightSource, _cameraPosition, gradient, color.rgb);
#endif

            // accomodate for variable sampling rates
            color.a = 1.0 - pow(1.0 - color.a, _samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);
            result.rgb = result.rgb + color.rgb * color.a  * (1.0 - result.a);
            result.a = result.a + (1.0 -result.a) * color.a;
        }

        // save first hit ray parameter for depth value calculation
        if (firstHitT < 0.0 && result.a > 0.0) {
            firstHitT = tNear;
//            out_FHP = vec4(samplePosition, 1.0);
//            out_FHN = vec4(normalize(computeGradient(_volume, _volumeTextureParams, samplePosition)), 1.0);
        }

        // early ray termination
        // if (result.a > 0.975) {
        //     result.a = 1.0;
        //     tNear = tFar;
        // 
        //         
        // }

        // advance to the next evaluation point along the ray
        tNear += _samplingStepSize;
        out_count.w += 1.0;
    }

         
    // calculate depth value from ray parameter
    gl_FragDepth = 1.0;
    if (firstHitT >= 0.0) {
        float depthEntry = texture(_entryPointsDepth, texCoords).z;
        float depthExit = texture(_exitPointsDepth, texCoords).z;
        gl_FragDepth = calculateDepthValue(firstHitT/tFar, depthEntry, depthExit);
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
    out_count = vec4(1.0, 0.5, 0.0, 1.0);

    //determine whether the ray has to be casted
    if (frontPos == backPos) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        out_Color = performRaycasting(frontPos, backPos, p);
    }
}
