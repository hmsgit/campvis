#include "tools/sampler2d.frag"
#include "tools/transferfunction.frag"

uniform sampler2D _texture;
uniform TextureParameters _textureParameters;

uniform sampler1D _tfTex;
uniform TFParameters _tfTextureParameters;

void main() {
    float intensity = getElement2DNormalized(_texture, _textureParameters, gl_TexCoord[0].xy).a;
    gl_FragData[0] = lookupTF(_tfTextureParameters, _tfTex, intensity);
}
