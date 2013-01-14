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

uniform Texture2D _firstColor;
uniform Texture2D _firstDepth;
uniform Texture2D _secondColor;
uniform Texture2D _secondDepth;

uniform int _compositingMethod;
uniform float _alpha;

void main() {
    if (_compositingMethod == 0) {
        // only first
        out_Color = getElement2DNormalized(_firstColor, ex_TexCoord.xy);
        gl_FragDepth = getElement2DNormalized(_firstDepth, ex_TexCoord.xy).z;
    }
    else if (_compositingMethod == 1) {
        // only second
        out_Color = getElement2DNormalized(_secondColor, ex_TexCoord.xy);
        gl_FragDepth = getElement2DNormalized(_secondDepth, ex_TexCoord.xy).z;
    }
    else if (_compositingMethod == 2) {
        // alpha blending
        vec4 firstColor = getElement2DNormalized(_firstColor, ex_TexCoord.xy);
        float firstDepth = getElement2DNormalized(_firstDepth, ex_TexCoord.xy).z;
        vec4 secondColor = getElement2DNormalized(_secondColor, ex_TexCoord.xy);
        float secondDepth = getElement2DNormalized(_secondDepth, ex_TexCoord.xy).z;

        out_Color = mix(firstColor, secondColor, _alpha);
        gl_FragDepth = min(firstDepth, secondDepth);
    }
    else if (_compositingMethod == 3) {
        // difference
        vec4 firstColor = getElement2DNormalized(_firstColor, ex_TexCoord.xy);
        float firstDepth = getElement2DNormalized(_firstDepth, ex_TexCoord.xy).z;
        vec4 secondColor = getElement2DNormalized(_secondColor, ex_TexCoord.xy);
        float secondDepth = getElement2DNormalized(_secondDepth, ex_TexCoord.xy).z;

        out_Color = vec4(vec3(1.0) - abs(firstColor - secondColor).xyz, max(firstColor.w, secondColor.w));
        gl_FragDepth = min(firstDepth, secondDepth);
    }
    else if (_compositingMethod == 4) {
        // depth test
        float firstDepth = getElement2DNormalized(_firstDepth, ex_TexCoord.xy).z;
        float secondDepth = getElement2DNormalized(_secondDepth, ex_TexCoord.xy).z;

        if (firstDepth > secondDepth) {
            out_Color = getElement2DNormalized(_secondColor, ex_TexCoord.xy);
            gl_FragDepth = secondDepth;
        }
        else {
            out_Color = getElement2DNormalized(_firstColor, ex_TexCoord.xy);
            gl_FragDepth = firstDepth;
        }
    }
    else {
        // should not occur, but makes the GLSL compiler happy
        out_Color = vec4(0.0);
    }

    if (out_Color.a == 0) {
        renderBackground(ex_TexCoord.xy, out_Color);
    }
}

