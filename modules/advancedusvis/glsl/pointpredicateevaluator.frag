// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
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
