// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

struct Texture2D {
    sampler2D _texture;
    vec2 _size;
    vec2 _sizeRCP;
};

/**
 * Texture lookup function for 2D textures using pixel coordinates, i.e [(0,0) , textureSize].
 * \param	tex			Texture2D struct with texture for lookup
 * \param	texCoords	Lookup coordinates in pixel coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement2D(in Texture2D tex, in vec2 texCoords) {
    vec2 texCoordsNormalized = texCoords * tex._sizeRCP;
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoordsNormalized, 0.0, 1.0)).xy;
    return texture2D(tex._texture, texCoordsNormalized);
}

/**
 * Texture lookup function for 2D textures using normalized texture coordinates, i.e. [0,1].
 * \param	tex			Texture2D struct with texture for lookup
 * \param	texCoords	Lookup coordinates in normlized texture coordinates
 * \return	The texel at the given coordinates.
 */
vec4 getElement2DNormalized(in Texture2D tex, in vec2 texCoords) {
    //vec2 texCoordsTransformed = (texParams.matrix_ * vec4(texCoords, 0.0, 1.0)).xy;
    return texture2D(tex._texture, texCoords);
}
