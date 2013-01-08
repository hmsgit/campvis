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


#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE colorTex0_;
uniform SAMPLER2D_TYPE depthTex0_;
uniform TEXTURE_PARAMETERS textureParameters0_;

uniform SAMPLER2D_TYPE colorTex1_;
uniform SAMPLER2D_TYPE depthTex1_;
uniform TEXTURE_PARAMETERS textureParameters1_;

uniform SAMPLER2D_TYPE colorTexDRRMax_;
uniform TEXTURE_PARAMETERS textureParametersDRRMax_;

uniform SAMPLER2D_TYPE colorTexDRRClipped_;
uniform TEXTURE_PARAMETERS textureParametersDRRClipped_;

/***
 * The main method.
 ***/
void main() {
    vec2 fragCoord = gl_FragCoord.xy;

    // fetch input textures
    vec4 color0 = textureLookup2Dscreen(colorTex0_, textureParameters0_, fragCoord);
    float depth0 = textureLookup2Dscreen(depthTex0_, textureParameters0_, fragCoord).z;
    vec4 color1 = textureLookup2Dscreen(colorTex1_, textureParameters1_, fragCoord);
    float depth1 = textureLookup2Dscreen(depthTex1_, textureParameters1_, fragCoord).z;
    
    float drrMax = textureLookup2Dscreen(colorTexDRRMax_, textureParametersDRRMax_, fragCoord).r;
    float drrClipped = textureLookup2Dscreen(colorTexDRRClipped_, textureParametersDRRClipped_, fragCoord).r;
    
	float weightingFactor = 0.0;
	if (drrClipped > 0)
		weightingFactor = clamp(drrClipped / drrMax, 0.0, 1.0);
	
    vec4 fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    float fragDepth = weightingFactor;
    
    if (weightingFactor == 0.0)
        fragDepth = depth1;
    else if (weightingFactor == 1.0)
        fragDepth = depth0;
    else
        fragDepth = min(depth0, depth1);
        
    fragColor = weightingFactor*color0 + (1.0-weightingFactor)*color1;

    FragData0 = fragColor;
    gl_FragDepth = fragDepth;
}
