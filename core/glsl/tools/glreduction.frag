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

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/texture2d.frag"

uniform sampler2D _texture;
uniform TextureParameters2D _textureParams;
uniform vec2 _texCoordsMultiplier;

void main() {
    vec2 tmp = ex_TexCoord.xy * _texCoordsMultiplier * 2.0;
    ivec2 texel = ivec2((tmp * _textureParams._size));

    vec4 a = texelFetch(_texture, texel, 0);
    vec4 b = texelFetch(_texture, texel + ivec2(1, 0), 0);
    vec4 c = texelFetch(_texture, texel + ivec2(0, 1), 0);
    vec4 d = texelFetch(_texture, texel + ivec2(1, 1), 0);

    //vec4 a = texture(_texture, tmp);
    //vec4 b = texture(_texture, texel + vec2(_textureParams._sizeRCP.x, 0));
    //vec4 c = texture(_texture, texel + vec2(0, _textureParams._sizeRCP.y));
    //vec4 d = texture(_texture, texel + vec2(_textureParams._sizeRCP.x, _textureParams._sizeRCP.y));

    //out_Color = max(a, max(b, max(c, d)));
    out_Color = a+b+c+d;
    //out_Color = vec4(texel, 0.0, 1.0);
    //out_Color = vec4(ex_TexCoord, 1.0);
}
