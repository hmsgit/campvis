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

#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

// 2D texture
uniform sampler2D _texture2d;
uniform TextureParameters2D _2dTextureParams;

// 3D texture
uniform sampler3D _texture3d;
uniform TextureParameters3D _3dTextureParams;

// Transfer Function
uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform bool _is3d;
uniform int _sliceNumber;
uniform vec4 _color;
uniform bool _isDepthTexture;

uniform bool _renderRChannel;
uniform bool _renderGChannel;
uniform bool _renderBChannel;
uniform bool _renderAChannel;

const vec4 checkerboardColor1 = vec4(0.90, 0.90, 0.90, 1.0);
const vec4 checkerboardColor2 = vec4(0.50, 0.50, 0.50, 1.0);

void main() {
    if (_is3d) {
        if (_sliceNumber < 0) {
            // perform MIP
            out_Color = vec4(0.0);
            for (float slice = _3dTextureParams._sizeRCP.z/2.0; slice < 1.0; slice += _3dTextureParams._sizeRCP.z) {
                out_Color = max(out_Color, lookupTF(_transferFunction, _transferFunctionParams, texture(_texture3d, vec3(ex_TexCoord.xy, slice)).r));
            }
        }
        else {
            // render the corresponding slice
            vec3 coord = vec3(ex_TexCoord.xy, (_sliceNumber * _3dTextureParams._sizeRCP.z) + (_3dTextureParams._sizeRCP.z / 2.0));
            vec4 texel = texture(_texture3d, coord);

            if (_3dTextureParams._numChannels == 1) {
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
            }
            else if (_3dTextureParams._numChannels == 3) {
                out_Color = vec4((abs(texel.rgb) - vec3(_transferFunctionParams._intensityDomain.x)) / (_transferFunctionParams._intensityDomain.y - _transferFunctionParams._intensityDomain.x), 1.0);
            }
            else if (_3dTextureParams._numChannels == 4) {
                out_Color = (abs(texel) - vec4(_transferFunctionParams._intensityDomain.x)) / (_transferFunctionParams._intensityDomain.y - _transferFunctionParams._intensityDomain.x);
            }
            else {
                out_Color = vec4(0.1, 0.6, 1.0, 0.75);
            }
            //out_Color = lookupTF(_transferFunction, _transferFunctionParams, texture(_texture3d, coord).r);
        }
    }
    else {
        vec4 texel = texture(_texture2d, ex_TexCoord.xy);
        if (_2dTextureParams._numChannels == 1) {
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
        }
        else if (_2dTextureParams._numChannels == 3) {
            out_Color = vec4(abs(texel.rgb), 1.0);
        }
        else if (_2dTextureParams._numChannels == 4) {
            out_Color = (abs(texel) - vec4(_transferFunctionParams._intensityDomain.x)) / (_transferFunctionParams._intensityDomain.y - _transferFunctionParams._intensityDomain.x);
        }
        else {
            out_Color = vec4(0.1, 0.6, 1.0, 0.75);
        }
    }

    if (! _renderRChannel)
        out_Color.r = 0.0;
    if (! _renderGChannel)
        out_Color.g = 0.0;
    if (! _renderBChannel)
        out_Color.b = 0.0;
    if (! _renderAChannel)
        out_Color.a = 1.0;

    // mix with fancy checkerboard pattern:
    if ((mod(ex_TexCoord.x * 10.0, 2.0) > 1.0) ^^ (mod(ex_TexCoord.y * 10.0, 2.0) > 1.0))
        out_Color = mix(checkerboardColor1, out_Color, out_Color.a);
    else
        out_Color = mix(checkerboardColor2, out_Color, out_Color.a);
}
