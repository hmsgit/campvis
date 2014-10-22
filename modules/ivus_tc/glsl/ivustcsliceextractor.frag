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
in vec4 ex_Color;
out vec4 out_Color;

#include "tools/colorspace.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform sampler3D _ivus;
uniform TextureParameters3D _ivusParams;
uniform sampler3D _cm;
uniform TextureParameters3D _cmParams;
uniform sampler3D _tc;
uniform TextureParameters3D _tcParams;
uniform sampler3D _plaque;
uniform TextureParameters3D _plaqueParams;

uniform mat4 _texCoordsMatrix;

uniform bool _mixWithIvus = true;
uniform bool _useTexturing = true;
uniform bool _useSolidColor = true;
uniform vec4 _color = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
    if (_useTexturing) {
        vec3 texel = (_texCoordsMatrix * vec4(ex_TexCoord, 1.0)).xyz;

        float ivus = texture(_ivus, texel).r;
        float cm = texture(_cm, texel).r;
        vec4 tc = texture(_tc, texel);
        float plaque = texture(_plaque, texel).r;

        vec4 pr = performPredicateBasedShading(ivus, cm, tc, plaque);

        vec3 hcy = vec3(pr.xy, cm * smoothstep(0.0, 0.5, pr.y));
        vec3 mixed = hcy2rgb(hcy);
        if (_mixWithIvus)
//            mixed = max(vec3(ivus), mixed);
            mixed = mix(vec3(ivus), mixed, max(cm, pr.y));
//            mixed = mix(vec3(ivus), mixed, min(cm, pr.y));

        //vec3 mixed = mix(vec3(1.0), vec3(1.0, 0.5, 0.0), max(cm, pr.y)); 
        out_Color = vec4(mixed, 1.0);

        //vec3 hsl = rgb2hcy(vec3(ivus));
        //hsl.xy += pr.xy;
        //hsl.x = mod(hsl.x, 1.0);
        //hsl.y *= cm;
        //out_Color = vec4(hcy2rgb(hsl), 1.0);
        
    }
    else if (_useSolidColor) {
        out_Color = _color;
    }
    else {
        out_Color = ex_Color;
    }
}
