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

in vec3 ex_TexCoord;
in vec4 ex_Color;
out vec4 out_Color;

//#include "tools/background.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform sampler3D _texture;
uniform TextureParameters3D _textureParams;

uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform mat4 _texCoordsMatrix;

uniform bool _useTexturing = true;
uniform bool _useSolidColor = true;
uniform vec4 _color = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
    if (_useTexturing) {
        vec4 texel = texture(_texture, (_texCoordsMatrix * vec4(ex_TexCoord, 1.0)).xyz);

        if (_textureParams._numChannels == 1) {
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
        }
        else if (_textureParams._numChannels == 3) {
            out_Color = vec4(abs(texel.rgb), 1.0);
        }
        else if (_textureParams._numChannels == 4) {
            out_Color = (abs(texel) - vec4(_transferFunctionParams._intensityDomain.x)) / (_transferFunctionParams._intensityDomain.y - _transferFunctionParams._intensityDomain.x);
        }
    }
    else if (_useSolidColor) {
        out_Color = _color;
    }
    else {
        out_Color = ex_Color;
    }
}
