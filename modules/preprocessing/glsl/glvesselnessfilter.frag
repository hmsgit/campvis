// ================================================================================================
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

in vec3 ex_TexCoord;

out float out_vesselness;
//out vec3 out_evals;

#include "tools/gradient.frag"
#include "tools/texture3d.frag"

uniform sampler3D _texture;
uniform TextureParameters3D _textureParams;

uniform sampler3D _labels;
uniform TextureParameters3D _labelsTexParams;
uniform int _labelBit = 3;

uniform float _zTexCoord;
uniform vec2 _lod = vec2(1.0, 3.0);

uniform float _alpha = 0.5;
uniform float _beta = 0.5;
uniform float _gamma = 0.001;
uniform float _theta = 0.5;

vec4 computeVesselness(in vec3 texCoords, in float lod) {
    if (true || texture(_texture, texCoords).rgb != vec3(0.0, 0.0, 0.0)) {
        mat3 hessian = computeHessianCentralDifferencesLod(_texture, _textureParams, texCoords, lod);
        vec3 evals = computeEigenvalues(hessian);

        if (abs(evals.x) < abs(evals.y)) {
            if (abs(evals.x) < abs(evals.z)) {
                evals.xyz = (abs(evals.y) < abs(evals.z)) ? evals.xyz : evals.xzy;
            }
            else {
                evals.xyz = evals.zxy;
            }
        }
        else {
            if (abs(evals.x) < abs(evals.z)) {
                evals.xyz = evals.yxz;
            }
            else {
                evals.xyz = (abs(evals.y) < abs(evals.z)) ? evals.yzx : evals.zyx;
            }
        }

        float l1 = evals.x;
        float l2 = evals.y;
        float l3 = evals.z;
    
        float rb = abs(l1) / sqrt(abs(l2 * l3));
        float ra = abs(l2) / abs(l3);
        float s = sqrt(l1 * l1   +   l2 * l2   +   l3 * l3);

        vec3 v3 = (hessian - mat3(l1)) * (hessian - mat3(l2))[0];

        float fbdir = abs(dot(normalize(v3), vec3(0, 1, 0)));

        float vesselness = 0.0;
        if (l2 < 0.0  && l3 < 0.0) {
            vesselness = (1.0 - exp(- (ra*ra)/(2.0 * _alpha * _alpha))) * exp(- (rb*rb)/(2.0 * _beta * _beta)) * (1.0 - exp(- (s*s)/(2.0 * _gamma * _gamma))) * (1.0 - exp(- (fbdir*fbdir)/(2.0 * _theta * _theta)));
        }

        //out_evals = vec3(fbdir);//max(out_evals, evals);
        return vec4(vesselness, ra, rb, s);
    }
    else {
        return vec4(0.0, 0.0, 0.0, 0.0);
    }
}


void main() {
    vec3 texCoords = vec3(ex_TexCoord.xy, _zTexCoord);

    //out_evals = vec3(1337.0);
    out_vesselness = 0.0;

    uint l = uint(texture(_labels, texCoords).r * 255);

    if (true || bitfieldExtract(l, 4, 1) != 0U) {
        for (float lod = _lod.x; lod < _lod.y; lod += 0.5) {
            vec4 level_vesselness = computeVesselness(texCoords, lod);
            out_vesselness = max(out_vesselness, level_vesselness.x);
        }
    }

}
