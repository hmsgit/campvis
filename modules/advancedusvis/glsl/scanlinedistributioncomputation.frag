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
