#include "tools/texture2d.frag"
#include "tools/transferfunction.frag"

uniform Texture2D _texture;

uniform sampler1D _tfTex;
uniform TFParameters _tfTextureParameters;

void main() {
    float intensity = getElement2DNormalized(_texture, gl_TexCoord[0].xy).a;
    gl_FragData[0] = lookupTF(_tfTextureParameters, _tfTex, intensity);
}
