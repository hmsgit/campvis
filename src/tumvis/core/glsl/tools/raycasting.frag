
/**
 * Code adapted from: https://www.marcusbannerman.co.uk/index.php/component/content/article/42-articles/97-vol-render-optimizations.htm
 */
void jitterEntryPoint(inout vec3 position, in vec3 direction, in float stepSize) {
    float random = fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453);
    position = position + direction * (stepSize * random);
}