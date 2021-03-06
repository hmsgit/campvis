// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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
out vec4 out_Color;

#include "tools/gradient.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform sampler3D _texture;
uniform TextureParameters3D _textureParams;

uniform sampler1D _transferFunction;
uniform TFParameters1D _transferFunctionParams;

uniform float _zTexCoord;
uniform int _numberOfBins;

void main() {
     vec4 intensity = texture(_texture, vec3(ex_TexCoord.xy, _zTexCoord));

     intensity.x = lookupTF(_transferFunction, _transferFunctionParams, intensity.x).a;
     intensity.y = lookupTF(_transferFunction, _transferFunctionParams, intensity.y).a;
     intensity.z = lookupTF(_transferFunction, _transferFunctionParams, intensity.z).a;
     intensity.w = lookupTF(_transferFunction, _transferFunctionParams, intensity.w).a;

     out_Color = floor(intensity * float(_numberOfBins + 1)) / float(_numberOfBins);
}
