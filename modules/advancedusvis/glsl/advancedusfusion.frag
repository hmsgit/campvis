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

#version 330

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/colorspace.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform sampler3D _usImage;
uniform TextureParameters3D _usTextureParams;

uniform sampler3D _blurredImage;
uniform TextureParameters3D _blurredTextureParams;

uniform sampler3D _confidenceMap;
uniform TextureParameters3D _confidenceTextureParams;

uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform int _sliceNumber;
uniform int _viewIndex;
uniform float _blurredScaling;

void main() {
    vec3 texCoord = vec3(ex_TexCoord.xy, _usTextureParams._sizeRCP.z * (_sliceNumber + 0.5));

    vec4 texel = getElement3DNormalized(_usImage, _usTextureParams, texCoord);
    vec4 blurred = getElement3DNormalized(_blurredImage, _blurredTextureParams, texCoord) * _blurredScaling;
    float confidence = getElement3DNormalized(_confidenceMap, _confidenceTextureParams, texCoord).a;

    switch (_viewIndex) {
        case 0:
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
            break;
        case 1:
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, blurred.a);
            break;
        case 2:
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a + texel.a - blurred.a);
            break;
        case 3:
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
            vec3 hsv = rgb2hsv(out_Color.xyz);
            hsv.x = 0.15;
            hsv.y = 1.0 - confidence;
            out_Color.xyz = hsv2rgb(hsv);
            break;
        case 4:
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
            vec3 hsl = rgb2hsl(out_Color.xyz);
            hsl.x = 0.15;
            hsl.y = 1.0 - confidence;
            out_Color.xyz = hsl2rgb(hsl);
            break;
        case 5:
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
            vec3 hcy = rgb2hcy(out_Color.xyz);
            hcy.x = 0.15;
            hcy.y = 1.0 - confidence;
            out_Color.xyz = hcy2rgb(hcy);
            break;
        case 6:
            float intensity = mix(blurred.a, 2.0 * texel.a - blurred.a, confidence);
            out_Color = lookupTF(_transferFunction, _transferFunctionParams, intensity);
            break;
    }
}
