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

#include "tools/shading.frag"
#include "tools/texture3d.frag"

in vec3 ex_TexCoord;        ///< incoming texture coordinate
in vec4 ex_Position;        ///< incoming texture coordinate

out vec4 out_Color;         ///< outgoing fragment color

uniform vec4 _color;
uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

uniform sampler3D _strainTexture;
uniform TextureParameters3D _textureParameters;

void main() {
    out_Color = _color;

    vec3 worldCoords = ex_Position.xyz / ex_Position.z;
    vec3 texCoords = worldToTexture(_textureParameters, worldCoords);
    out_Color = getElement3DNormalized(_strainTexture, _textureParameters, texCoords);

    while (length(out_Color) == 0.0) {
        worldCoords -= normalize(ex_TexCoord) * 0.1;
        texCoords = worldToTexture(_textureParameters, worldCoords);
        out_Color = getElement3DNormalized(_strainTexture, _textureParameters, texCoords);
    }

#ifdef ENABLE_SHADING
    // compute gradient (needed for shading and normals)
    vec3 gradient = ex_TexCoord;
    out_Color.rgb = calculatePhongShading(ex_Position.xyz / ex_Position.z, _lightSource, _cameraPosition, gradient, _color.rgb, _color.rgb, vec3(1.0, 1.0, 1.0));
#endif
}