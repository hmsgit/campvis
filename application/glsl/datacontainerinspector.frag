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

#version 330

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/texture2d.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform Texture2D _texture2d;
uniform Texture3D _texture3d;
uniform TransferFunction1D _transferFunction;
uniform bool _is3d;
uniform int _sliceNumber;
uniform vec4 _color;

const vec4 checkerboardColor1 = vec4(0.90, 0.90, 0.90, 1.0);
const vec4 checkerboardColor2 = vec4(0.50, 0.50, 0.50, 1.0);

void main() {
    if (_is3d) {
        if (_sliceNumber < 0) {
            // perform MIP
            out_Color = vec4(0.0);
            for (float slice = 0.0; slice < 1.0; slice += _texture3d._sizeRCP.z) {
                out_Color = max(out_Color, lookupTF(_transferFunction, getElement3DNormalized(_texture3d, vec3(ex_TexCoord.xy, slice)).a));
            }
        }
        else {
            // render the corresponding slice
            vec3 coord = vec3(ex_TexCoord.xy, (_sliceNumber + 0.5) / (_texture3d._size.z));
            out_Color = lookupTF(_transferFunction, getElement3DNormalized(_texture3d, coord).a);
        }
    }
    else {
        vec4 texel = getElement2DNormalized(_texture2d, ex_TexCoord.xy);
        if (_texture2d._numChannels == 1) {
            out_Color = lookupTF(_transferFunction, texel.a);
        }
        else if (_texture2d._numChannels == 3) {
            out_Color = vec4(abs(texel.rgb), 1.0);
        }
        else if (_texture2d._numChannels == 4) {
            out_Color = (abs(texel) - vec4(_transferFunction._intensityDomain.x)) / (_transferFunction._intensityDomain.y - _transferFunction._intensityDomain.x);
        }
        else {
            out_Color = vec4(0.1, 0.6, 1.0, 0.75);
        }
    }

    // mix with fancy checkerboard pattern:
    if ((mod(ex_TexCoord.x * 10.0, 2.0) > 1.0) ^^ (mod(ex_TexCoord.y * 10.0, 2.0) > 1.0))
        out_Color = mix(checkerboardColor1, out_Color, out_Color.a);
    else
        out_Color = mix(checkerboardColor2, out_Color, out_Color.a);
}
