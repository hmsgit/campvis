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

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/colorspace.frag"
#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

#ifdef USE_3D_TEX
#define SAMPLER_TYPE sampler3D
#define TEXPARAMS_TYPE TextureParameters3D
#define TEXTURE_LOOKUP_FUNC texture
#else
#define SAMPLER_TYPE sampler2D
#define TEXPARAMS_TYPE TextureParameters2D
#define TEXTURE_LOOKUP_FUNC texture
#endif

uniform SAMPLER_TYPE _usImage;
uniform TEXPARAMS_TYPE _usTextureParams;

uniform SAMPLER_TYPE _blurredImage;
uniform TEXPARAMS_TYPE _blurredTextureParams;

uniform SAMPLER_TYPE _confidenceMap;
uniform TEXPARAMS_TYPE _confidenceTextureParams;

uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform sampler1D _confidenceTF;
uniform TFParameters1D _confidenceTFParams;

uniform int _sliceNumber;
uniform int _viewIndex;
uniform float _confidenceScaling;
uniform float _hue;
uniform float _blurredScale;

void main() {
#ifdef USE_3D_TEX
    vec3 texCoord = vec3(ex_TexCoord.xy, _usTextureParams._sizeRCP.z * (_sliceNumber + 0.5));
#else
    vec2 texCoord = ex_TexCoord.xy;
#endif

    vec4 texel = TEXTURE_LOOKUP_FUNC(_usImage, texCoord);
    vec4 blurred = TEXTURE_LOOKUP_FUNC(_blurredImage, texCoord) * _blurredScale;
    float confidence = clamp(TEXTURE_LOOKUP_FUNC(_confidenceMap, texCoord).r * _confidenceScaling, 0.0, 1.0);
    float uncertainty = lookupTF(_confidenceTF, _confidenceTFParams, confidence).a;

    if (confidence <= 0.0) {
        out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
    }
    else {
        switch (_viewIndex) {
            case 0:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
                break;
            case 1:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, blurred.r);
                break;
            case 2:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, confidence);
                break;
            case 3:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
                vec3 hsv = rgb2hsv(out_Color.xyz);
                hsv.x = _hue;
                hsv.y = uncertainty;
                out_Color.xyz = hsv2rgb(hsv);
                break;
            case 4:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
                vec3 hsl = rgb2hsl(out_Color.xyz);
                hsl.x = _hue;
                hsl.y = uncertainty;
                out_Color.xyz = hsl2rgb(hsl);
                break;
            case 5:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
                vec3 tsl = rgb2tsl(out_Color.xyz);
                //tsl.x = _hue;
                //tsl.y = uncertainty;
                out_Color.xyz = tsl2rgb(tsl);
                break;
            case 6:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
                vec3 hcl = rgb2hcl(out_Color.xyz);
                hcl.x = _hue;
                hcl.y = uncertainty;
                out_Color.xyz = hcl2rgb(hcl);
                break;
            case 7:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
                vec3 hcy = rgb2hcy(out_Color.xyz);
                hcy.x = _hue;
                hcy.y = uncertainty;
                out_Color.xyz = hcy2rgb(hcy);
                break;
            case 8:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
                vec3 lch = rgb2lch(out_Color.xyz);
                lch.z = 6.2831853 * _hue;
                lch.y = 100.0 * (uncertainty);
                out_Color.xyz = lch2rgb(lch);
                break;
            case 9:
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, texel.r);
                vec3 hlch = lab2lch(xyz2hlab(rgb2xyz(out_Color.xyz)));
                //hlch.z = 6.2831853 * _hue;
                //hlch.y = 100.0 * (uncertainty);
                out_Color.xyz = xyz2rgb(hlab2xyz(lch2lab(hlch)));
                break;
            case 10:
                float intensity = mix((2.0 * texel.r - blurred.r), blurred.r, uncertainty);
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, intensity);
                break;
            case 11:
                float lod = max(floor((uncertainty) * 6.0), 0.0);

                vec4 lodTexel = texture(_usImage, texCoord, lod);
                out_Color = lookupTF(_transferFunction, _transferFunctionParams, lodTexel.r);   
                break;
        }
    }
}
