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

// XOR Bitmask texture
uniform usampler2D _xorBitmask;

const int MAXSTEPS = 20;
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

bool intersectBits(usampler2D vhTexture, in uvec4 bitRay, in ivec2 texel, in int level, out uvec4 intersectionBitmask)
{
    // Fetch bitmask from hierarchy and compute intersection via bitwise AND
    intersectionBitmask = (bitRay & texelFetch(vhTexture, texel, level));
    return (intersectionBitmask != uvec4(0));
}

bool intersectHierarchy(usampler2D vhTexture, int vhMaxMipmapLevel, in vec3 origin, in vec3 direction, in int level, in vec3 posTNear, out float tFar, out uvec4 intersectionBitmask) {
    // Calculate pixel coordinates ([0,width]x[0,height])
    // of the current position along the ray
    float res = float(1 << (vhMaxMipmapLevel - level));
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
        vhTexture,
        texture(_xorBitmask, vec2(min(posTNear.z, zFar), max(posTNear.z, zFar))),
        pixelCoord, 
        level, 
        intersectionBitmask);
}

float clipFirstHitpoint(usampler2D vhTexture, int vhMaxMipmapLevel, in vec3 origin, in vec3 direction, in float tNear, in float tFar) {
    // clip start/endpoint to bounding box
    vec2 clipPoints = intersectAABB(origin, direction, vec3(0.0), vec3(1.0));
    if (clipPoints.x > clipPoints.y)
        return tFar;

    // Set current position along the ray to the ray’s origin
    vec3 posTNear = origin;
    bool intersectionFound = false;
    uvec4 intersectionBitmask = uvec4(0);

    // It’s faster to not start at the coarsest level
    int level = vhMaxMipmapLevel / 2;
    for (int i = 0; (i < MAXSTEPS) && (tNear < tFar) && (!intersectionFound); i++) {
        float newTFar = 1.0;
        if (intersectHierarchy(vhTexture, vhMaxMipmapLevel, origin, direction, level, posTNear, newTFar, intersectionBitmask)) {
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

    return clamp(tNear, 0.0, tFar);
}
