
#include "tools/texture2d.frag"

uniform vec2 _viewportSize;
uniform vec2 _viewportSizeRCP;
uniform Texture2D _colorTexture;
uniform Texture2D _depthTexture;

void main() {
    vec2 fragCoord = gl_FragCoord.xy * _viewportSizeRCP;
    gl_FragData[0] = getElement2DNormalized(_colorTexture, fragCoord);
    gl_FragDepth = getElement2DNormalized(_depthTexture, fragCoord).z;
}
