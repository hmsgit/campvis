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
#include "tools/background.frag"

uniform sampler2D _normalColor;
uniform sampler2D _normalDepth;
uniform TextureParameters2D _normalTexParams;

uniform sampler2D _mirrorColor;
uniform sampler2D _mirrorDepth;
uniform TextureParameters2D _mirrorTexParams;

uniform sampler2D _mirrorRenderedDepth;
uniform TextureParameters2D _mirrorRenderedTexParams;

void main() {
    float normalDepth = getElement2DNormalized(_normalDepth, _normalTexParams, ex_TexCoord.xy).z;
    float mirrorRenderedDepth = getElement2DNormalized(_mirrorRenderedDepth, _mirrorRenderedTexParams, ex_TexCoord.xy).z;

    if (normalDepth <= mirrorRenderedDepth) {
        out_Color = getElement2DNormalized(_normalColor, _normalTexParams, ex_TexCoord.xy);
        gl_FragDepth = normalDepth;
    }
    else {
        out_Color = getElement2DNormalized(_mirrorColor, _mirrorTexParams, ex_TexCoord.xy);
        gl_FragDepth = getElement2DNormalized(_mirrorDepth, _mirrorTexParams, ex_TexCoord.xy).z;
    }

    if (out_Color.a == 0) {
        renderBackground(ex_TexCoord.xy, out_Color);
    }
}

