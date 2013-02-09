// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

struct TextureParameters2D {
    vec2 _size;
    vec2 _sizeRCP;
    int _numChannels;
};

/**
 * Texture lookup function for 2D textures using pixel coordinates, i.e [(0,0) , textureSize].
 * \param	tex			Sampler to texture
 * \param	texParams   TextureParameters2D struct with texture for lookup
 * \param	texCoords	Lookup coordinates in pixel coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement2D(in sampler2D tex, in TextureParameters2D texParams, in vec2 texCoords) {
    vec2 texCoordsNormalized = texCoords * texParams._sizeRCP;
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoordsNormalized, 0.0, 1.0)).xy;
    return texture(tex, texCoordsNormalized);
}

/**
 * Texture lookup function for 2D textures using normalized texture coordinates, i.e. [0,1].
 * \param	tex			Sampler to texture
 * \param	texParams   TextureParameters2D struct with texture for lookup
 * \param	texCoords	Lookup coordinates in normlized texture coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement2DNormalized(in sampler2D tex, in TextureParameters2D texParams, in vec2 texCoords) {
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
    return texture(tex, texCoords);
}
