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

#include "tools/shading.frag"
#include "tools/texture2d.frag"

// input from geometry shader
in vec3 geom_Position;
in vec3 geom_TexCoord;
in vec4 geom_Color;
in vec3 geom_Normal;
in vec4 geom_Picking;
noperspective in vec3 geom_EdgeDistance;

// output fragment color
out vec4 out_Color;
out vec4 out_Picking;

// additional uniforms
uniform int _coloringMode;
uniform vec4 _solidColor;
uniform vec4 _wireframeColor;
uniform float _lineWidth = 1.0;

uniform LightSource _lightSource;
uniform vec3 _cameraPosition;

#ifdef ENABLE_TEXTURING
uniform sampler2D _texture;
uniform TextureParameters2D _textureParams;
#endif

void main() {
    out_Color = vec4(1.0, 0.5, 0.0, 1.0);

#ifdef ENABLE_TEXTURING
    out_Color = texture(_texture, geom_TexCoord.xy);
#else
    if (_coloringMode == 0)
        out_Color = geom_Color;
    else if (_coloringMode == 1)
        out_Color = _solidColor;
#endif        

#ifdef ENABLE_SHADING
    // perform Phong shading
    out_Color.rgb = calculatePhongShading(geom_Position, _lightSource, _cameraPosition, geom_Normal, out_Color.rgb, out_Color.rgb, vec3(1.0, 1.0, 1.0));
#endif

#ifdef WIREFRAME_RENDERING
    // Find the smallest distance to the edges
    float d = min(geom_EdgeDistance.x, min(geom_EdgeDistance.y, geom_EdgeDistance.z)) * 2.0;

    // perform anti-aliasing
    float mixVal = smoothstep(_lineWidth - 1.0, _lineWidth + 1.0, d);

    // Mix the surface color with the line color
    out_Color = mix(_wireframeColor, out_Color, mixVal);    
#endif

    out_Picking = geom_Picking;

}
