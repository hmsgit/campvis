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

// TODO: implement coordinate transformation using a trafo matrix?

struct TextureParameters3D {
    // Texture size
    vec3 _size;
    vec3 _sizeRCP;

    // Number of channels
    int _numChannels;

    // Voxel spacing
    vec3 _voxelSize;
    vec3 _voxelSizeRCP;

    // Transformation matrices
    mat4 _textureToWorldMatrix;
    mat4 _worldToTextureMatrix;

    vec2 _realWorldMapping;
};

/**
 * Texture lookup function for 3D textures using voxel coordinates, i.e [(0,0) , textureSize].
 * \param	tex			Sampler to texture
 * \param	texParams   TextureParameters3D struct with texture for lookup
 * \param	texCoords	Lookup coordinates in pixel coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement3D(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords) {
    vec3 texCoordsNormalized = texCoords * texParams._sizeRCP;
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoordsNormalized, 0.0, 1.0)).xy;
    return texture(tex, texCoordsNormalized);
}

/**
 * Texture lookup function for 3D textures using normalized texture coordinates, i.e. [0,1].
 * \param	tex			Sampler to texture
 * \param	texParams   TextureParameters3D struct with texture for lookup
 * \param	texCoords	Lookup coordinates in normlized texture coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement3DNormalized(in sampler3D tex, in TextureParameters3D texParams, in vec3 texCoords) {
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
    return texture(tex, texCoords);
}

/**
 * Transforms texture coordinates for texture \a tex to world coordinates using the texture's
 * texture-to-world matrix.
 * \param	texParams   TextureParameters3D struct with texture for lookup
 * \param   texCoords   texture coordinates
 * \return  \a texCoords transformes to woorld coordinates.
 */
vec4 textureToWorld(in TextureParameters3D texParams, in vec3 texCoords) {
    return texParams._textureToWorldMatrix * vec4(texCoords, 1.0);
}

/**
 * Transforms world coordinates for texture \a tex to texture coordinates using the texture's
 * world-to-texture matrix.
 * \param	texParams   TextureParameters3D struct with texture for lookup
 * \param   worldCoords world coordinates
 * \return  \a texCoords transformes to texture coordinates.
 */
vec4 worldToTexture(in TextureParameters3D texParams, in vec3 worldCoords) {
    return texParams._worldToTextureMatrix * vec4(worldCoords, 1.0);
}

float applyRealWorldMapping(in TextureParameters3D tex, in float value) {
    return (value + tex._realWorldMapping.x) * tex._realWorldMapping.y;
}

float applyInverseRealWorldMapping(in TextureParameters3D tex, in float value) {
    return (value - tex._realWorldMapping.x) / tex._realWorldMapping.y;
}
