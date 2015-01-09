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

#include "tools/gradient.frag"
#include "tools/texture3d.frag"
#include "tools/transferfunction.frag"

uniform sampler3D _texture;
uniform TextureParameters3D _textureParams;

uniform ivec3 _outputSize;
uniform ivec3 _offset;
uniform int _zTexel;

void main() {
    //ivec2 texel = ivec2(((1.0 / vec2(_outputSize.xy)) + ex_TexCoord.xy) * vec2(_outputSize.xy));
    ivec2 texel = ivec2(ex_TexCoord.xy * vec2(_outputSize.xy));
    vec4 intensity = texelFetch(_texture, ivec3(texel, _zTexel) + _offset, 0);
    out_Color = intensity;
}
