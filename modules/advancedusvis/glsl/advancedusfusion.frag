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

#include "tools/colorspace.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform Texture3D _usImage;
uniform Texture3D _confidenceMap;
uniform Texture3D _gradientMap;
uniform TransferFunction1D _transferFunction;

uniform int _sliceNumber;

void main() {
    vec3 texCoord = vec3(ex_TexCoord.xy, _usImage._sizeRCP.z * (_sliceNumber + 0.5));

    vec4 texel = getElement3DNormalized(_usImage, texCoord);
    vec4 gradient = getElement3DNormalized(_gradientMap, texCoord);
    float confidence = getElement3DNormalized(_confidenceMap, texCoord).a;

    out_Color = lookupTF(_transferFunction, texel.a);

    vec3 hsv = rgb2hsv(out_Color.xyz);
    hsv.y = 1.0 - confidence;
    out_Color.xyz = hsv2rgb(hsv);
}
