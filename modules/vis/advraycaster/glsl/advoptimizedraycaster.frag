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
uniform usampler2D _vvTexture;
uniform TextureParameters2D _vvTextureParams;
uniform int _vvVoxelSize;
uniform int _vvVoxelDepth;
uniform bool _hasVv;
uniform int _vvMaxMipMapLevel;


uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform float _samplingStepSize;

#ifdef INTERSECTION_REFINEMENT
bool _inVoid = false;
#endif

#ifdef ENABLE_SHADOWING
uniform float _shadowIntensity;
#endif

const float positiveInfinity = 1.0 / 0.0;

// TODO: copy+paste from Voreen - eliminate or improve.
const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

// converting the sample coordinates [0, 1]^3 to integer voxel coordinate in l-level of the voxelized texture. 
ivec3 voxelToBrick(in vec3 voxel, int level) {
    int res = 1 << level;

    return ivec3(floor(voxel.x / (_vvVoxelSize * res)), floor(voxel.y / (_vvVoxelSize * res)), floor(voxel.z / _vvVoxelDepth));
}

// Looks up the l mipmap level of voxelized texture of the render data and returns the value in samplePosition coordinates.
// samplePosition is in texture coordiantes [0, 1]
// level is the level in the hierarchy
uint lookupInBbv(in vec3 samplePosition, in int level) {
    ivec3 byte = voxelToBrick(samplePosition * _volumeTextureParams._size, level);
    uint texel = uint(texelFetch(_vvTexture, byte.xy, level).r);

    return texel;
}

// Converts the ray to bitmask which can be intersect with voxelized volume
void rayMapToMask(in vec3 samplePos, in vec3 entryPoint, in vec3 direction, in int level, out uint mask, inout float tFar) {
    int res = 1 << level ;							//< the number of voxels from voxelized volume data which are mapped into 
                                                    //< voxelized volume in l-th level of hierarchy 

    // calculating the voxel which the ray's origin will start from.
    // BoxLlf is the lower-left corner of the voxel.
    // BoxUrb is the upper-right corner of the voxel.
    // NOTE: the z-direction coordinate is not important. So, we simply put BoxLlf.z = 0 and BoxUrb.z = 1. Because we are 
    //		 going to intersect the raybitmask with the bitmask of the voxel in z-direction to find the intersection point.
    vec3 brickVoxel = floor((samplePos * _volumeTextureParams._size) / vec3(_vvVoxelSize * res, _vvVoxelSize * res, _vvVoxelDepth)) * vec3(_vvVoxelSize * res, _vvVoxelSize * res, _vvVoxelDepth);
    vec3 boxLlf = vec3((brickVoxel * _volumeTextureParams._sizeRCP).xy, 0.0f);
    vec3 boxUrb = vec3((boxLlf + (_volumeTextureParams._sizeRCP * vec3(_vvVoxelSize * res, _vvVoxelSize * res, _vvVoxelDepth))).xy, 1.0f);

    // here, tmin and tmax for the ray which is intersecting with the voxel is computed. 
    vec3 tMin = (boxLlf - entryPoint) / direction;
    vec3 tMax = (boxUrb - entryPoint) / direction;

    // As tmin and tmax values are not necessary tmin and tmax values and value should be resorted, to find the 
    // exit point of the voxel we just compute minimum value of maximum values in tmin and tmax.
    vec3 tFarVec = max(tMin, tMax);
    tFar = min(tFar, min(tFarVec.x, min(tFarVec.y, tFarVec.z)));

    // Here we are going to calculate the entry and exit point from the voxel by calculated tFar.
    int bit1 = clamp(int(floor(samplePos.z * 32)), 0, 31);
    int bit2 = clamp(int(floor((entryPoint.z + tFar * direction.z) * 32)), 0, 31);

    // setting the bits of the bitmask values.
    mask = bitfieldInsert(uint(0x0), uint(0xffffffff), max((min(bit1, bit2)), 0), min(abs(bit2 - bit1) + 1, 32));
    
    // HACK HACK HACK
    // right now, the bitmask generation for ray is not working correctly.
    mask = 0xffffffff;
}

// intersects the ray bitmask and voxelized data bitmap.
// bitray: ray bitmask,
// texel: the texel coordinate in the voxelized volume data,
// level: the level of the texture,
// intersectionBitmask: the result bitmask of intersecting the data in texel of the voxelized volume data and ray bitmask.
// it returns wherther there was an intersection or not.
bool intersectBits(in uint bitRay, in vec3 texel, in int level, out uint intersectionBitmask) {
   // Fetch bitmask from hierarchy and compute intersection via bitwise AND
   intersectionBitmask = (bitRay & lookupInBbv(texel, level));
   return (intersectionBitmask != uint(0));
}

// 
bool intersectHierarchy(in int level, in vec3 samplePos, in vec3 entryPoint, in vec3 direction, inout float tFar, out uint intersectionBitmask) {
   uint rayBitmask = uint(0);

   // Mapping the ray to a bitmask
   rayMapToMask(samplePos, entryPoint, direction, level, rayBitmask, tFar);

   // Intersect the ray bitmask with current pixel
   return intersectBits(rayBitmask, samplePos, level, intersectionBitmask);
}

/**
 * Performs the raycasting and returns the final fragment color.
 */
vec4 performRaycasting(in vec3 entryPoint, in vec3 exitPoint, in vec2 texCoords) {
    vec4 result = vec4(0.0);
    vec3 direction = exitPoint - entryPoint;
    float tNear = 0.0;
    float tFar = length(direction);
    direction = normalize(direction);

    jitterEntryPoint(entryPoint, direction, _samplingStepSize * _jitterStepSizeMultiplier);

    float firstHitT = -1.0f;
  
 
    // compute sample position
    vec3 samplePosition = entryPoint.rgb + tNear * direction;

    bool intersectionFound = false;
    uint intersectionBitmask = uint(0);
    int level = min(3, _vvMaxMipMapLevel);			//< start from a level of hierarchy. We are trying to start from one of the levels in middle (not coarse and not best).
    float offset = (0.0025 / int(1 << int(_vvMaxMipMapLevel))) / length(direction);	//< offset value used to avoid self-intersection or previous voxel intersection.
    int i = 0;
    float newTfar = 1.0f;							//< tFar calculated for the point where the ray is going out of current texel in voxelized data.
    while(!intersectionFound && (tNear <= tFar) && (level >= 0) && (level < _vvMaxMipMapLevel) && (i < 100)) {
        newTfar = tFar;								//< because it will be compared to calculated tFar in intersectHierarchy function.

        // intersects the ray with the hierarchy of voxelized volume texture.
        if(intersectHierarchy(level, samplePosition, entryPoint, direction, newTfar, intersectionBitmask)) {
            // if the level is 0, the intersection is found, otherwise check the intersection in a higher level of hierarchy.
            intersectionFound = (level == 0);               
            level--;
        } else {
            // The beginning part of the ray is cut and going to a coarser level of hierarchy.
            tNear = newTfar + offset;
            samplePosition = entryPoint + tNear * direction;
            level++;                
        }

        i++;
    }

              
    if(!intersectionFound) {
        result = vec4(0, 0, 0, 0);
        return result;
    }

    bool tFarIntersectionFound = false;
    uint tFarIntersectionBitmask = uint(0);
    int tFarLevel = min(3, _vvMaxMipMapLevel);
    int k = 0;
    float tFarDec = 0.0;
    float newTnear = 1.0f;
    vec3 tFarSamplePosition = exitPoint - tFarDec * direction;
    while(!tFarIntersectionFound && (tFarDec <= tFar) && (tFarLevel >= 0) && (tFarLevel < _vvMaxMipMapLevel) && (k < 100)) {
        newTnear = tFar;

        if(intersectHierarchy(tFarLevel, tFarSamplePosition, exitPoint, -direction, newTnear, tFarIntersectionBitmask)) {
            tFarIntersectionFound = (tFarLevel == 0);               
            tFarLevel--;
        } else {
            tFarDec = newTnear + offset;
            tFarSamplePosition = exitPoint - tFarDec * direction;
            tFarLevel++;

                
        }

        k++;
    }
              
    if(!tFarIntersectionFound) {
        result = vec4(0, 0, 0, 0);
        return result;
    }

    tFar -= tFarDec;

    while (tNear < tFar) {
        vec3 samplePosition = entryPoint.rgb + tNear * direction;

        // lookup intensity and TF
        float intensity = texture(_volume, samplePosition).r;
        vec4 color = lookupTF(_transferFunction, _transferFunctionParams, intensity);

#ifdef INTERSECTION_REFINEMENT
        if (color.a <= 0.0) {
            // we're within void, make the steps bigger
            _inVoid = true;
        } else {
            if (_inVoid) {
                float formerT = t - _samplingStepSize;

                // we just left the void, perform intersection refinement
                for (float stepPower = 0.5; stepPower > 0.1; stepPower /= 2.0) {
                    // compute refined sample position
                    float newT = formerT + _samplingStepSize * stepPower;
                    vec3 newSamplePosition = entryPoint.rgb + newT * direction;

                    // lookup refined intensity + TF
                    float newIntensity = texture(_volume, newSamplePosition).r;
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
        tNear += _samplingStepSize;
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

    //determine whether the ray has to be casted
    if (frontPos == backPos) {
        //background need no raycasting
        discard;
    } else {
        //fragCoords are lying inside the boundingbox
        out_Color = performRaycasting(frontPos, backPos, p);
    }
}
