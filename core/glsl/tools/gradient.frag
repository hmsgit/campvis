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
