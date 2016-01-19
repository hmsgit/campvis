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

#include "tools/texture2d.frag"

in vec3 ex_TexCoord;
out vec4 out_Color;

uniform sampler2D _image1;
uniform TextureParameters2D _image1Params;

uniform sampler2D _image2;
uniform TextureParameters2D _image2Params;

#define HALF_WINDOW_SIZE 4
#define WINDOW_SIZE 9
#define WINDOW_SIZE_SQ 81


void main() {
    vec4 toReturn = vec4(0.0);

    const float L = 128;
    const float c1 = (0.01 * L) * (0.01 * L);
    const float c2 = (0.03 * L) * (0.03 * L);

    vec4 xMean = vec4(0);
    vec4 yMean = vec4(0);
    vec4 xM2   = vec4(0);
    vec4 yM2   = vec4(0);
    vec4 cov   = vec4(0);
    float n = 0;

    vec4 mean1 = vec4(0);
    vec4 mean2 = vec4(0);
    vec4 M12   = vec4(0);
    float NN = WINDOW_SIZE_SQ;

    vec2 shift = _image1Params._sizeRCP * HALF_WINDOW_SIZE;
    float mult = _image1Params._sizeRCP;

    for (int i = 0; i <= WINDOW_SIZE_SQ; ++i) {
        vec2 iOffset = (vec2(i % WINDOW_SIZE, i / WINDOW_SIZE) * mult) - shift;
        vec4 xi = texture(_image1, ex_TexCoord.xy + iOffset);
        vec4 yi = texture(_image2, ex_TexCoord.xy + iOffset);

        // compute mean and variance with Knuth's algorithm 
        // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
        n = n+1;

        vec4 xDelta = xi - xMean;
        xMean += xDelta/n;
        xM2 += xDelta * (xi - xMean);

        vec4 yDelta = yi - yMean;
        yMean += yDelta/n;
        yM2 += yDelta * (yi - yMean);



        // compute the covariance
        vec4 delta1 = (xi - mean1) / n;
        mean1 += delta1;
        vec4 delta2 = (yi - mean2) / n;
        mean2 += delta2;
        M12 += n * delta1 * delta2 - M12 / n;

    }

    cov = NN / (NN - 1) * M12;
    vec4 xVar = xM2 / (n-1);
    vec4 yVar = yM2 / (n-1);
    vec4 ssim = ((2*xMean*yMean + c1) * (2*cov + c2)) / ((xMean * xMean + yMean * yMean + c1) * (xVar + yVar + c2));

    out_Color = min(vec4(1.0), vec4(ssim));
}
