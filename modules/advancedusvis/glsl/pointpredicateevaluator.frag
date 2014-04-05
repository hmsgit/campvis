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
out uint out_Mask;

#include "tools/gradient.frag"
#include "tools/texture3d.frag"


// volume
uniform sampler3D _volume;
uniform TextureParameters3D _volumeTextureParams;

uniform sampler3D _labels;
uniform TextureParameters3D _labelsParams;
uniform sampler3D _snr;
uniform TextureParameters3D _snrParams;
uniform sampler3D _vesselness;
uniform TextureParameters3D _vesselnessParams;
uniform sampler3D _confidence;
uniform TextureParameters3D _confidenceParams;

uniform float _zTexCoord;
uniform vec3 _scanningDirection = vec3(0.0, 1.0, 0.0);
uniform vec3 _cameraPosition = vec3(1.0, 1.0, 1.0);

void main() {
    vec3 position = vec3(ex_TexCoord.xy, _zTexCoord); 

    float intensity = texture(_volume, position).x;
    uint label = uint(texture(_labels, position).x * 255);
    float snr = texture(_snr, position).x;
    float vesselness = texture(_vesselness, position).x;
    float confidence = texture(_confidence, position).x;

    vec3 gradient = computeGradientCentralDifferencesLod(_volume, _volumeTextureParams, position, 0.5);
    float gradientAngle = acos(dot(normalize(gradient), normalize(_scanningDirection))) * 57.29577951308;
    vec3 worldPosition = textureToWorld(_volumeTextureParams, position).xyz;
    float viewAngle = acos(dot(normalize(gradient), normalize(worldPosition - _cameraPosition))) * 57.29577951308;

    //out_Mask = 0xFFFFFFFF;
    out_Mask = computePredicateBitfield(intensity, length(gradient) * 10.0, gradientAngle, viewAngle, snr, vesselness, confidence, label);
}
