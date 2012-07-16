
#include "tools/sampler2d.frag"

uniform sampler2D _colorTexture;
uniform sampler2D _depthTexture;
uniform TextureParameters _textureParameters;

void main() {
    vec2 fragCoord = gl_FragCoord.xy * screenDimRCP_;
    gl_FragData[0] = getElement2DNormalized(_colorTexture, _textureParameters, fragCoord);
    gl_FragDepth = getElement2DNormalized(_depthTexture, _textureParameters, fragCoord).z;
}
