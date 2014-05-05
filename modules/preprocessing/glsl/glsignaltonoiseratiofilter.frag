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
out float out_result;

#include "tools/gradient.frag"
#include "tools/texture3d.frag"

uniform sampler3D _texture;
uniform TextureParameters3D _textureParams;

uniform float _zTexCoord;


void sampleOffset(inout float sum, inout float sumsq, inout int count, in float referenceValue, const in ivec3 offset) {
    float value = abs(referenceValue - textureOffset(_texture, vec3(ex_TexCoord.xy, _zTexCoord), offset).r);

    sum += value;
    sumsq += value * value;
    ++count;
}

void main() {
    float refVal = texture(_texture, vec3(ex_TexCoord.xy, _zTexCoord)).r;

    float sum = refVal;
    float sumsq = refVal * refVal;
    int count = 1;
    
    sampleOffset(sum, sumsq, count, refVal, ivec3(-1,  0,  0));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 1,  0,  0));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 0, -1,  0));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 0,  1,  0));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 0,  0, -1));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 0,  0,  1));

    sampleOffset(sum, sumsq, count, refVal, ivec3(-2, -2, -2));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 2, -2, -2));
    sampleOffset(sum, sumsq, count, refVal, ivec3(-2,  2, -2));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 2,  2, -2));
    sampleOffset(sum, sumsq, count, refVal, ivec3(-2, -2,  2));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 2, -2,  2));
    sampleOffset(sum, sumsq, count, refVal, ivec3(-2,  2,  2));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 2,  2,  2));
    
    sampleOffset(sum, sumsq, count, refVal, ivec3(-3,  0,  0));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 3,  0,  0));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 0, -3,  0));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 0,  3,  0));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 0,  0, -3));
    sampleOffset(sum, sumsq, count, refVal, ivec3( 0,  0,  3));


    float mean = sum / float(count);
    float sdev = sqrt( (1.0 / float(count - 1)) * (sumsq - (sum * sum / float(count))));

    out_result = (sdev != 0.0) ? mean / sdev : 0.0;
}
