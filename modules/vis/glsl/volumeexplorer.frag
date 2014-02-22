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

#include "tools/background.frag"
#include "tools/texture2d.frag"

uniform sampler2D _colorTexture;
uniform sampler2D _depthTexture;
uniform TextureParameters2D _texParams;

uniform bool _renderBackground = false;

void main() {
    out_Color = texture(_colorTexture, ex_TexCoord.xy);
    gl_FragDepth = texture(_depthTexture, ex_TexCoord.xy).r;

    if (_renderBackground) {
        out_Color = blendBackground(ex_TexCoord.xy, out_Color);
    }
}