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
 
#version 330

in vec3 ex_TexCoord;
out vec4 out_Color;

#include "tools/texture2d.frag"
#include "tools/background.frag"

uniform Texture2D _xRayColor;
uniform Texture2D _xRayDepth;
uniform Texture2D _sliceColor;
uniform Texture2D _sliceDepth;

uniform Texture2D _drrFullColor;
uniform Texture2D _drrClippedColor;

/***
 * The main method.
 ***/
void main() {
    // fetch input textures
    vec4 xRayColor = getElement2DNormalized(_xRayColor, ex_TexCoord.xy);
    float xRayDepth = getElement2DNormalized(_xRayDepth, ex_TexCoord.xy).z;
    vec4 sliceColor = getElement2DNormalized(_sliceColor, ex_TexCoord.xy);
    float sliceDepth = getElement2DNormalized(_sliceDepth, ex_TexCoord.xy).z;
    
    float drrFull = getElement2DNormalized(_drrFullColor, ex_TexCoord.xy).r;
    float drrClipped = getElement2DNormalized(_drrClippedColor, ex_TexCoord.xy).r;
    
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
    //gl_FragDepth = fragDepth;
}
