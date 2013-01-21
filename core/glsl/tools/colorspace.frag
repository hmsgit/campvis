// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

/**
 * Converts a color from RGB space to HSV space.
 * Using the efficient implementation from http://chilliant.blogspot.de/2010/11/rgbhsv-in-hlsl.html.
 * \param   colorRGB    color in RGB space
 * \return  The given color in HSV space.
 */
vec3 rgb2hsv(vec3 colorRGB) {
    vec3 HSV = vec3(0.0);

    HSV.z = max(colorRGB.r, max(colorRGB.g, colorRGB.b));
    float M = min(colorRGB.r, min(colorRGB.g, colorRGB.b));
    float difference = HSV.z - M;

    if (difference != 0) {
        HSV.y = difference / HSV.z;
        vec3 delta = (HSV.z - colorRGB) / difference;
        delta.rgb -= delta.brg;
        delta.rg += vec2(2.0, 4.0);

        if (colorRGB.r >= HSV.z)
            HSV.x = delta.b;
        else if (colorRGB.g >= HSV.z)
            HSV.x = delta.r;
        else
            HSV.x = delta.g;

        HSV.x = fract(HSV.x / 6.0);
    }

    return HSV;
}


/**
 * Converts a color from HSV space to RGB space.
 * Using the efficient implementation from http://chilliant.blogspot.de/2010/11/rgbhsv-in-hlsl.html.
 * \param   colorHSV    color in HSV space.
 * \return  The given color in RGB space.
 */
vec3 hsv2rgb(vec3 colorHSV) {
    float R = abs(colorHSV.x * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(colorHSV.x * 6.0 - 2.0);
    float B = 2.0 - abs(colorHSV.x * 6.0 - 4.0);

    vec3 toReturn = clamp(vec3(R, G, B), 0.0, 1.0);
    return ((toReturn - 1.0) * colorHSV.y + 1.0) * colorHSV.z;
}


/**
 * Converts a color from RGB space to CIEXYZ space.
 * \see     http://wiki.labomedia.org/images/1/10/Orange_Book_-_OpenGL_Shading_Language_2nd_Edition.pdf
 * \param   colorRGB    color in RGB space
 * \return  The given color in CIEXYZ space.
 */
vec3 rgb2ciexyz(in vec3 colorRGB) {
    const mat3 conversionMatrix = mat3(
        0.412453, 0.212671, 0.019334,
        0.357580, 0.715160, 0.119193,
        0.180423, 0.072169, 0.950227);
    return colorRGB * conversionMatrix;
}

/**
 * Converts a color from CIEXYZ space to RGB space.
 * \see     http://wiki.labomedia.org/images/1/10/Orange_Book_-_OpenGL_Shading_Language_2nd_Edition.pdf
 * \param   colorCIEXYZ   color in CIEXYZ space
 * \return  The given color in RGB space.
 */
vec3 ciexyz2rgb(in vec3 colorCIEXYZ) {
    const mat3 conversionMatrix = mat3(
        3.240479, -0.969256, 0.055648,
        -1.537150, 1.875992, -0.204043,
        -0.498535, 0.041556, 1.057311);
    return colorCIEXYZ * conversionMatrix;
}
