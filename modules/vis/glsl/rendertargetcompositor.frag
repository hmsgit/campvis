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
out vec4 out_Color;

#include "tools/texture2d.frag"
#include "tools/background.frag"

uniform sampler2D _firstColor;
uniform sampler2D _firstDepth;
uniform TextureParameters2D _firstTexParams;

uniform sampler2D _secondColor;
uniform sampler2D _secondDepth;
uniform TextureParameters2D _secondTexParams;

uniform int _compositingMethod;
uniform float _alpha;
uniform bool _enableBackground;

void main() {
    float depth = 1.0;

    if (_compositingMethod == 0) {
        // only first
        out_Color = texture(_firstColor, ex_TexCoord.xy);
        depth = texture(_firstDepth, ex_TexCoord.xy).r;
    }
    else if (_compositingMethod == 1) {
        // only second
        out_Color = texture(_secondColor, ex_TexCoord.xy);
        depth = texture(_secondDepth, ex_TexCoord.xy).r;
    }
    else if (_compositingMethod == 2) {
        // alpha blending
        vec4 firstColor = texture(_firstColor, ex_TexCoord.xy);
        float firstDepth = texture(_firstDepth, ex_TexCoord.xy).r;
        vec4 secondColor = texture(_secondColor, ex_TexCoord.xy);
        float secondDepth = texture(_secondDepth, ex_TexCoord.xy).r;

        out_Color = mix(firstColor, secondColor, _alpha);
        depth = min(firstDepth, secondDepth);
    }
    else if (_compositingMethod == 3) {
        // difference
        vec4 firstColor = texture(_firstColor, ex_TexCoord.xy);
        float firstDepth = texture(_firstDepth, ex_TexCoord.xy).r;
        vec4 secondColor = texture(_secondColor, ex_TexCoord.xy);
        float secondDepth = texture(_secondDepth, ex_TexCoord.xy).r;

        out_Color = vec4(vec3(1.0) - abs(firstColor - secondColor).xyz, max(firstColor.w, secondColor.w));
        depth = min(firstDepth, secondDepth);
    }
    else if (_compositingMethod == 4) {
        // depth test
        vec4 firstColor = texture(_firstColor, ex_TexCoord.xy);
        float firstDepth = texture(_firstDepth, ex_TexCoord.xy).r;
        vec4 secondColor = texture(_secondColor, ex_TexCoord.xy);
        float secondDepth = texture(_secondDepth, ex_TexCoord.xy).r;

        if (firstDepth > secondDepth) {
            out_Color = mix(firstColor, secondColor, secondColor.a);
            depth = secondDepth;
        }
        else {
            out_Color = mix(secondColor, firstColor, firstColor.a);
            depth = firstDepth;
        }
    }
    else {
        // should not occur, but makes the GLSL compiler happy
        out_Color = vec4(0.0);
    }

    if (_enableBackground)
        out_Color = blendBackground(ex_TexCoord.xy, out_Color);

    gl_FragDepth = depth;
}

