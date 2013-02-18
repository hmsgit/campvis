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
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

#ifdef USE_3D_TEX
#define SAMPLER_TYPE sampler3D
#define TEXPARAMS_TYPE TextureParameters3D
#define TEXTURE_LOOKUP_FUNC getElement3DNormalized
#else
#define SAMPLER_TYPE sampler2D
#define TEXPARAMS_TYPE TextureParameters2D
#define TEXTURE_LOOKUP_FUNC getElement2DNormalized
#endif

uniform SAMPLER_TYPE _usImage;
uniform TEXPARAMS_TYPE _usTextureParams;

uniform SAMPLER_TYPE _blurredImage;
uniform TEXPARAMS_TYPE _blurredTextureParams;

uniform SAMPLER_TYPE _confidenceMap;
uniform TEXPARAMS_TYPE _confidenceTextureParams;

uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform int _sliceNumber;
uniform int _viewIndex;
uniform float _confidenceScaling;
uniform float _hue;


void main() {
#ifdef USE_3D_TEX
    vec3 texCoord = vec3(ex_TexCoord.xy, _usTextureParams._sizeRCP.z * (_sliceNumber + 0.5));
#else
    vec2 texCoord = ex_TexCoord.xy;
#endif

    vec4 texel = TEXTURE_LOOKUP_FUNC(_usImage, _usTextureParams, texCoord);
    vec4 blurred = TEXTURE_LOOKUP_FUNC(_blurredImage, _blurredTextureParams, texCoord);
    float confidence = clamp(TEXTURE_LOOKUP_FUNC(_confidenceMap, _confidenceTextureParams, texCoord).a * _confidenceScaling, 0.0, 1.0);

    if (confidence <= 0.0) {
        out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
    }
    else {
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
                hsv.x = _hue;
                hsv.y = 1.0 - confidence;
                out_Color.xyz = hsv2rgb(hsv);
                break;
            case 4:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
                vec3 hsl = rgb2hsl(out_Color.xyz);
                hsl.x = _hue;
                hsl.y = 1.0 - confidence;
                out_Color.xyz = hsl2rgb(hsl);
                break;
            case 5:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
                vec3 tsl = rgb2tsl(out_Color.xyz);
                //tsl.x = _hue;
                //tsl.y = 1.0 - confidence;
                out_Color.xyz = tsl2rgb(tsl);
                break;
            case 6:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
                vec3 hcl = rgb2hcl(out_Color.xyz);
                hcl.x = _hue;
                hcl.y = 1.0 - confidence;
                out_Color.xyz = hcl2rgb(hcl);
                break;
            case 7:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
                vec3 hcy = rgb2hcy(out_Color.xyz);
                hcy.x = _hue;
                hcy.y = 1.0 - confidence;
                out_Color.xyz = hcy2rgb(hcy);
                break;
            case 8:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.a);
                vec3 lab = rgb2lch(out_Color.xyz);
                //lab.y = 100.0 * _hue * (1.0 - confidence) - 50.0;
                //lab.z = 100.0 * _hue * (1.0 - confidence);
                out_Color.xyz = lch2rgb(lab);
                break;
            case 9:
                float intensity = mix(blurred.a, (2.0 * texel.a - blurred.a), confidence);
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, intensity);
                break;
            case 10:
                float lod = max(floor((1.0 - confidence) * 6.0), 0.0);

                vec4 lodTexel = texture(_usImage, texCoord, lod);
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, lodTexel.a);
                break;
        }
    }
}
