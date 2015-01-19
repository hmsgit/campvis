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
out vec4 out_Color;

#ifdef GLRESAMPLER_3D
uniform sampler3D _texture;
uniform float _zTexCoord;
#endif

#ifdef GLRESAMPLER_2D
uniform sampler2D _texture;
#endif

void main() {
	#ifdef GLRESAMPLER_3D
    vec4 sample = texture(_texture, vec3(ex_TexCoord.xy, _zTexCoord));
    #endif

    #ifdef GLRESAMPLER_2D
    vec4 sample = texture(_texture, ex_TexCoord.xy);
    #endif

    out_Color = sample;
}
