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

uniform sampler2D _normalColor;
uniform sampler2D _normalDepth;
uniform TextureParameters2D _normalTexParams;

uniform sampler2D _mirrorColor;
uniform sampler2D _mirrorDepth;
uniform TextureParameters2D _mirrorTexParams;

uniform sampler2D _mirrorRenderedDepth;
uniform TextureParameters2D _mirrorRenderedTexParams;

void main() {
    float normalDepth = texture(_normalDepth, ex_TexCoord.xy).r;
    float mirrorRenderedDepth = texture(_mirrorRenderedDepth, ex_TexCoord.xy).r;

    if (normalDepth <= mirrorRenderedDepth) {
        out_Color = texture(_normalColor, ex_TexCoord.xy);
        gl_FragDepth = normalDepth;
    }
    else {
        out_Color = texture(_mirrorColor, ex_TexCoord.xy);
        gl_FragDepth = texture(_mirrorDepth, ex_TexCoord.xy).r;
    }

    if (out_Color.a == 0) {
        renderBackground(ex_TexCoord.xy, out_Color);
    }
}

