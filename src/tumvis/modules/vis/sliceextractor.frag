
#include "tools/sampler2d.frag"

uniform sampler2D _texture;
uniform TextureParameters _textureParameters;

void main() {
    float intensity = getElement2DNormalized(_texture, _textureParameters, gl_TexCoord[0].xy).a * 10.0;
    gl_FragData[0] = vec4(intensity, intensity, intensity, 1.0);
}
