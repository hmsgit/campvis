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

/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2012 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/texture2d.frag"
#include "tools/background.frag"

uniform sampler2D _xRayColor;
uniform sampler2D _xRayDepth;
uniform TextureParameters2D _xRayTexParams;

uniform sampler2D _sliceColor;
uniform sampler2D _sliceDepth;
uniform TextureParameters2D _sliceTexParams;

uniform sampler2D _drrFullColor;
uniform TextureParameters2D _drrFullTexParams;

uniform sampler2D _drrClippedColor;
uniform TextureParameters2D _drrClippedTexParams;

/***
 * The main method.
 ***/
void main() {
    // fetch input textures
    vec4 xRayColor = texture(_xRayColor, ex_TexCoord.xy);
    float xRayDepth = texture(_xRayDepth, ex_TexCoord.xy).r;
    vec4 sliceColor = texture(_sliceColor, ex_TexCoord.xy);
    float sliceDepth = texture(_sliceDepth, ex_TexCoord.xy).r;
    
    float drrFull = texture(_drrFullColor, ex_TexCoord.xy).r;
    float drrClipped = texture(_drrClippedColor, ex_TexCoord.xy).r;
    
    float weightingFactor = 0.0;
    if (drrClipped > 0)
        weightingFactor = clamp(drrClipped / drrFull, 0.0, 1.0);
    
    float fragDepth = weightingFactor;
    
    if (weightingFactor == 0.0)
        fragDepth = sliceDepth;
    else if (weightingFactor == 1.0)
        fragDepth = xRayDepth;
    else
        fragDepth = min(xRayDepth, sliceDepth);
        
    out_Color = weightingFactor*xRayColor + (1.0-weightingFactor)*sliceColor;
    gl_FragDepth = fragDepth;

    if (out_Color.a == 0) {
        renderBackground(ex_TexCoord.xy, out_Color);
    }
}
