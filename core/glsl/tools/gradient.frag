#include "tools/texture3d.frag"

/**
 * Compute the gradient using forward differences on the texture's alpha channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientForwardDifferences(in Texture3D tex, in vec3 texCoords) {
    vec3 offset = tex._sizeRCP;
    float v = getElement3DNormalized(tex, texCoords).a;
    float dx = getElement3DNormalized(tex, texCoords + vec3(offset.x, 0.0, 0.0)).a;
    float dy = getElement3DNormalized(tex, texCoords + vec3(0, offset.y, 0)).a;
    float dz = getElement3DNormalized(tex, texCoords + vec3(0, 0, offset.z)).a;
    return vec3(v - dx, v - dy, v - dz) * tex._voxelSize;
}

/**
 * Compute the gradient using central differences on the texture's alpha channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientCentralDifferences(in Texture3D tex, in vec3 texCoords) {
    vec3 offset = tex._sizeRCP;
    float dx = getElement3DNormalized(tex, texCoords + vec3(offset.x, 0.0, 0.0)).a;
    float dy = getElement3DNormalized(tex, texCoords + vec3(0, offset.y, 0)).a;
    float dz = getElement3DNormalized(tex, texCoords + vec3(0, 0, offset.z)).a;
    float mdx = getElement3DNormalized(tex, texCoords + vec3(-offset.x, 0, 0)).a;
    float mdy = getElement3DNormalized(tex, texCoords + vec3(0, -offset.y, 0)).a;
    float mdz = getElement3DNormalized(tex, texCoords + vec3(0, 0, -offset.z)).a;
    return vec3(mdx - dx, mdy - dy, mdz - dz) * tex._voxelSize * vec3(0.5);
}


/**
 * Compute the gradient using filtered central differences on the texture's alpha channel.
 * \param   tex         3D texture to calculate gradients for
 * \param   texCoords   Lookup position in texture coordinates
 */
vec3 computeGradientFilteredCentralDifferences(in Texture3D tex, in vec3 texCoords) {
    vec3 offset = tex._sizeRCP;
    vec3 g0 = computeGradientCentralDifferences(tex, texCoords);
    vec3 g1 = computeGradientCentralDifferences(tex, texCoords + vec3(-offset.x, -offset.y, -offset.z));
    vec3 g2 = computeGradientCentralDifferences(tex, texCoords + vec3( offset.x,  offset.y,  offset.z));
    vec3 g3 = computeGradientCentralDifferences(tex, texCoords + vec3(-offset.x,  offset.y, -offset.z));
    vec3 g4 = computeGradientCentralDifferences(tex, texCoords + vec3( offset.x, -offset.y,  offset.z));
    vec3 g5 = computeGradientCentralDifferences(tex, texCoords + vec3(-offset.x, -offset.y,  offset.z));
    vec3 g6 = computeGradientCentralDifferences(tex, texCoords + vec3( offset.x,  offset.y, -offset.z));
    vec3 g7 = computeGradientCentralDifferences(tex, texCoords + vec3(-offset.x,  offset.y,  offset.z));
    vec3 g8 = computeGradientCentralDifferences(tex, texCoords + vec3( offset.x, -offset.y, -offset.z));
    vec3 mix0 = mix(mix(g1, g2, 0.5), mix(g3, g4, 0.5), 0.5);
    vec3 mix1 = mix(mix(g5, g6, 0.5), mix(g7, g8, 0.5), 0.5);
    return mix(g0, mix(mix0, mix1, 0.5), 0.75);
}
