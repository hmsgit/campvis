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

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/gradient.frag"
#include "tools/texture3d.frag"
#include "tools/raycasting.frag"

uniform sampler3D _texture;
uniform TextureParameters3D _textureParams;

uniform float _zTexCoord;

uniform vec3 _scanningDirection;
uniform float _stepSize = 7.0;
uniform float _intensityThreshold = 0.2;


struct TraversalInfo {
    float thisIntensity;
    float interfaceDistance;
    float thatIntensity;
};

float computeLocalIntensity(in vec3 position, in vec3 singleStep) {
    float sum = 0.0;
    //sum += texture(_texture, position - (3.0/7.0) * singleStep).r;
    //sum += texture(_texture, position - (2.0/7.0) * singleStep).r;
    //sum += texture(_texture, position - (1.0/7.0) * singleStep).r;
    sum += texture(_texture, position).r;
    //sum += texture(_texture, position + (1.0/7.0) * singleStep).r;
    //sum += texture(_texture, position + (2.0/7.0) * singleStep).r;
    //sum += texture(_texture, position + (3.0/7.0) * singleStep).r;

    return sum / 7.0;
}

// traverse along ray until find a region with a local intensity differing more from this region's
// intensity than threshold
float traverse(in vec3 start, in vec3 stepDirection, in float intensityThreshold) {
    float toReturn = -1.0;
    float thisIntensity = texture(_texture, start).r;
    vec3 samplePosition = start;

    // compute end of ray (where the volume ends)
    float stepSize = length(stepDirection);
    float t = stepSize;
    float maxT = rayBoxIntersection(start, stepDirection, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), t);

    if (maxT == positiveInfinity) {
        maxT = 0.0;
        toReturn = 42.0;
    }

    while (t < maxT) {
        // compute sample position, local intensity and the average intensity of the current region
        samplePosition = start + (t * stepDirection);
        float intensity = texture(_texture, samplePosition).r;

        if (abs(thisIntensity - intensity) > intensityThreshold) {
            // we found a new region, update return value and exit
            return distance(start, samplePosition);
        }

        t += stepSize;
    }

    return distance(start, samplePosition);
}


void main() {
    vec3 position = vec3(ex_TexCoord.xy, _zTexCoord);
    vec3 stepDirection = normalize(_scanningDirection * _textureParams._voxelSize) * length(_textureParams._sizeRCP) * 2;
    
    float forward = traverse(position, stepDirection, _intensityThreshold);
    float backward = traverse(position, -stepDirection, _intensityThreshold);

    out_Color.x = forward + length(_textureParams._sizeRCP);
    out_Color.y = backward + length(_textureParams._sizeRCP);
    out_Color.z = out_Color.x + out_Color.y;

    //out_Color.xyz = stepDirection;
}
