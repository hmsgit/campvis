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

// The weights of RGB contributions to luminance.
// Should sum to unity.
const vec3 HCYwts_ = vec3(0.299, 0.587, 0.114);

/**
 * Converts pure Hue to RGB
 * Using the efficient implementation from http://www.chilliant.com/rgb2hsv.html.
 * \param   H   Pure hue
 * \return  The pure hue converted to RGB.
 */
vec3 hue2rgb(in float H) {
    float R = abs(H * 6.0 - 3.0) - 1.0;
    float G = 2.0 - abs(H * 6.0 - 2.0);
    float B = 2.0 - abs(H * 6.0 - 4.0);
    return clamp(vec3(R,G,B), 0.0, 1.0);
}

/**
 * Converts RGB color plus chromacity and value to hue value.
 * Using the efficient implementation from http://www.chilliant.com/rgb2hsv.html.
 * \param   RGB     color in RGB space
 * \param   C       Chromacity
 * \param   V       Value
 * \return  The corresponding hue.
 */
float rgbcv2hue(in vec3 RGB, in float C, in float V) {
    vec3 delta = (V - RGB) / C;
    delta.rgb -= delta.brg;
    delta.rgb += vec3(2.0, 4.0, 6.0);
    delta.brg = step(V, RGB) * delta.brg;
    float H;
    H = max(delta.r, max(delta.g, delta.b));
    return fract(H / 6.0);
}

/**
 * Converts a color from HSV space to RGB space.
 * Using the efficient implementation from http://www.chilliant.com/rgb2hsv.html.
 * \param   HSV    color in HSV space
 * \return  The given color in RGB space.
 */
vec3 hsv2rgb(in vec3 HSV) {
    vec3 RGB = hue2rgb(HSV.x);
    return ((RGB - 1.0) * HSV.y + 1.0) * HSV.z;
}

/**
 * Converts a color from HSL space to RGB space.
 * Using the efficient implementation from http://www.chilliant.com/rgb2hsv.html.
 * \param   HSL    color in HSL space
 * \return  The given color in RGB space.
 */
vec3 hsl2rgb(in vec3 HSL) {
    vec3 RGB = hue2rgb(HSL.x);
    float C = (1.0 - abs(2.0 * HSL.z - 1.0)) * HSL.y;
    return (RGB - 0.5) * C + HSL.z;
}
   
/**
 * Converts a color from HCY space to RGB space.
 * Using the efficient implementation from http://www.chilliant.com/rgb2hsv.html.
 * \param   HCY    color in HCY space
 * \return  The given color in RGB space.
 */
vec3 hcy2rgb(in vec3 HCY) {
    vec3 RGB = hue2rgb(HCY.x);
    float Z = dot(RGB, HCYwts_);
    if (HCY.z < Z) {
        HCY.y *= HCY.z / Z;
    }
    else if (Z < 1.0) {
        HCY.y *= (1.0 - HCY.z) / (1 - Z);
    }
    return (RGB - vec3(Z, Z, Z)) * HCY.y + HCY.z;
}

/**
 * Converts a color from RGB space to HSV space.
 * Using the efficient implementation from http://www.chilliant.com/rgb2hsv.html.
 * \param   RGB    color in RGB space
 * \return  The given color in HSV space.
 */
vec3 rgb2hsv(in vec3 RGB) {
    vec3 HSV = vec3(0.0, 0.0, 0.0);
    HSV.z = max(RGB.r, max(RGB.g, RGB.b));
    float M = min(RGB.r, min(RGB.g, RGB.b));
    float C = HSV.z - M;
    if (C != 0) {
        HSV.x = rgbcv2hue(RGB, C, HSV.z);
        HSV.y = C / HSV.z;
    }
    return HSV;
}

/**
 * Converts a color from RGB space to HSL space.
 * Using the efficient implementation from http://www.chilliant.com/rgb2hsv.html.
 * \param   RGB    color in RGB space
 * \return  The given color in HSL space.
 */
vec3 rgb2hsl(in vec3 RGB) {
    vec3 HSL = vec3(0.0, 0.0, 0.0);
    float U, V;
    U = -min(RGB.r, min(RGB.g, RGB.b));
    V = max(RGB.r, max(RGB.g, RGB.b));
    HSL.z = (V - U) * 0.5;
    float C = V + U;
    if (C != 0) {
        HSL.x = rgbcv2hue(RGB, C, V);
        HSL.y = C / (1.0 - abs(2.0 * HSL.z - 1.0));
    }
    return HSL;
}


/**
 * Converts a color from RGB space to HCY space.
 * Using the efficient implementation from http://www.chilliant.com/rgb2hsv.html.
 * \param   RGB    color in RGB space
 * \return  The given color in HCY space.
 */
vec3 rgb2hcy(in vec3 RGB) {
    vec3 HCY = vec3(0.0, 0.0, 0.0);
    float U, V;
    U = -min(RGB.r, min(RGB.g, RGB.b));
    V = max(RGB.r, max(RGB.g, RGB.b));
    HCY.y = V + U;
    HCY.z = dot(RGB, HCYwts_);
    if (HCY.y != 0) {
        HCY.x = rgbcv2hue(RGB, HCY.y, V);
        float Z = dot(hue2rgb(HCY.x), HCYwts_);
        if (HCY.z > Z) {
            HCY.z = 1 - HCY.z;
            Z = 1 - Z;
        }
        HCY.y *= Z / HCY.z;
    }
    return HCY;
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
