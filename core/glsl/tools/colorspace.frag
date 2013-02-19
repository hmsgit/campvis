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
const vec3 colorspace_HcyWts_ = vec3(0.299, 0.587, 0.114);
const vec3 colorspace_LabWts_ = vec3(95.0456, 100.0, 108.8754);

const float colorspace_pi_ = 3.14159265;
const float colorspace_two_pi_ = 6.2831853;

float colorspace_HclGamma_ = 3.0;
float colorspace_HclY0_ = 100;
float colorspace_HclMaxL_ = 0.530454533953517;

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
    float Z = dot(RGB, colorspace_HcyWts_);
    if (HCY.z < Z) {
        HCY.y *= HCY.z / Z;
    }
    else if (Z < 1.0) {
        HCY.y *= (1.0 - HCY.z) / (1 - Z);
    }
    return (RGB - vec3(Z, Z, Z)) * HCY.y + HCY.z;
}


vec3 hcl2rgb(in vec3 HCL)
{
  vec3 RGB = vec3(0.0, 0.0, 0.0);
  if (HCL.z != 0.0) {
    float H = HCL.x;
    float C = HCL.y;
    float L = HCL.z * colorspace_HclMaxL_;
    float Q = exp((1.0 - C / (2.0 * L)) * (colorspace_HclGamma_ / colorspace_HclY0_));
    float U = (2.0 * L - C) / (2.0 * Q - 1.0);
    float V = C / Q;
    float T = tan((H + min(fract(2.0 * H) / 4.0, fract(-2.0 * H) / 8.0)) * 6.283185307);
    H *= 6.0;
    if (H <= 1.0) {
      RGB.r = 1.0;
      RGB.g = T / (1.0 + T);
    }
    else if (H <= 2.0) {
      RGB.r = (1.0 + T) / T;
      RGB.g = 1.0;
    }
    else if (H <= 3.0) {
      RGB.g = 1.0;
      RGB.b = 1.0 + T;
    }
    else if (H <= 4.0) {
      RGB.g = 1.0 / (1.0 + T);
      RGB.b = 1.0;
    }
    else if (H <= 5.0) {
      RGB.r = -1.0 / T;
      RGB.b = 1.0;
    }
    else {
      RGB.r = 1.0;
      RGB.b = -T;
    }
    return RGB * V + U;
  }
  return RGB;
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
    HCY.z = dot(RGB, colorspace_HcyWts_);
    if (HCY.y != 0) {
        HCY.x = rgbcv2hue(RGB, HCY.y, V);
        float Z = dot(hue2rgb(HCY.x), colorspace_HcyWts_);
        if (HCY.z > Z) {
            HCY.z = 1 - HCY.z;
            Z = 1 - Z;
        }
        HCY.y *= Z / HCY.z;
    }
    return HCY;
}

vec3 rgb2hcl(in vec3 RGB) {
  vec3 HCL = vec3(0.0, 0.0, 0.0);
  float H = 0.0;
  float U, V;
  U = -min(RGB.r, min(RGB.g, RGB.b));
  V = max(RGB.r, max(RGB.g, RGB.b));
  float Q = colorspace_HclGamma_ / colorspace_HclY0_;
  HCL.y = V + U;
  if (HCL.y != 0.0)
  {
    H = atan(RGB.g - RGB.b, RGB.r - RGB.g) / 3.14159265;
    Q *= -U / V;
  }
  Q = exp(Q);
  HCL.x = fract(H / 2.0 - min(fract(H), fract(-H)) / 6.0);
  HCL.y *= Q;
  HCL.z = mix(U, V, Q) / (colorspace_HclMaxL_ * 2.0);
  return HCL;
}

vec3 rgb2tsl(in vec3 RGB) {
    const float twoPiRCP_ = 0.15915494309;
    const float oneThird = 1.0 / 3.0;
    vec3 TSL = vec3(0.0, 0.0, 0.0);
    float sum = RGB.r + RGB.g + RGB.b;
    float rs = (RGB.r / sum) - oneThird;
    float gs = (RGB.g / sum) - oneThird;

    if (gs > 0)
        TSL.x = twoPiRCP_ * atan(rs/gs + 0.25);
    else if (gs < 0)
        TSL.x = twoPiRCP_ * atan(rs/gs + 0.75);

    TSL.y = sqrt(9.0 * (rs*rs + gs*gs) / 5.0);

    TSL.z = dot(RGB, colorspace_HcyWts_);
    return TSL;
};

vec3 tsl2rgb(in vec3 TSL) {
    vec3 RGB = vec3(0.0, 0.0, 0.0);
    float x = -1.0 / tan(TSL.x * 6.283185307);
    
    if (TSL.x == 0.0)
        RGB.g = 0.0;
    else if (TSL.x > 0.5)
        RGB.g = TSL.y * -sqrt(5.0/(9.0 * (x*x + 1.0)));
    else if (TSL.x < 0.5)
        RGB.g = TSL.y * sqrt(5.0/(9.0 * (x*x + 1.0)));
    
    if (TSL.x == 0)
        RGB.r = TSL.y * sqrt(5.0)/3.0;
    else
        RGB.r = x * RGB.g + (1.0/3.0);

    RGB.b = 1.0 - RGB.r - RGB.g;
    float k = 1.0 / (0.185*RGB.r + 0.473*RGB.b + 0.114);
    return k * RGB;
}

// ================================================================================================

/**
 * Converts a color from RGB space to CIE XYZ space.
 * \see     http://easyrgb.com
 * \param   RGB    color in RGB space
 * \return  The given color in CIE XYZ space.
 */
vec3 rgb2xyz(in vec3 RGB) {
    bvec3 tmp = greaterThan(RGB, vec3(0.04045));
    RGB =   (vec3(tmp)      * pow((RGB + 0.055) / 1.055, vec3(2.4))) 
          + (vec3(not(tmp)) * (RGB / 12.92));

    RGB *= 100.0;

    const mat3 conversionMatrix = mat3(
        0.412453, 0.357580, 0.180423,
        0.212671, 0.715160, 0.072169,
        0.019334, 0.119193, 0.950227);
    return RGB * conversionMatrix;
}

/**
 * Converts a color from CIE XYZ space to RGB space.
 * \see     http://easyrgb.com
 * \param   XYZ   color in CIE XYZ space
 * \return  The given color in RGB space.
 */
vec3 xyz2rgb(in vec3 XYZ) {
    XYZ /= 100.0;

    const mat3 conversionMatrix = mat3(
        3.240479, -1.537150, -0.498535,
        -0.969256, 1.875992, 0.041556,
        0.055648, -0.204043, 1.057311);

    bvec3 tmp = greaterThan(XYZ, vec3(0.0031308));
    XYZ =   (vec3(tmp)      * (1.055 * pow(XYZ, vec3(1.0/2.4)) - 0.055))
          + (vec3(not(tmp)) * (XYZ * 12.92));
    
    return XYZ;
}

// ================================================================================================

/**
 * Converts a color from XYZ space to L*a*b* space.
 * \see     http://easyrgb.com
 * \param   XYZ    color in XYZ space
 * \return  The given color in L*a*b* space.
 */
vec3 xyz2lab(in vec3 XYZ) {
    XYZ /= colorspace_LabWts_;

    bvec3 mask = greaterThan(XYZ, vec3(0.008856));
    XYZ =   (vec3(mask)      * pow(XYZ, vec3(1.0/3.0)))
          + (vec3(not(mask)) * (7.787 * XYZ + 16.0/116.0));

    return vec3(116.0 * XYZ.y - 16.0,
                500.0 * (XYZ.x - XYZ.y),
                200.0 * (XYZ.y - XYZ.z));
}

/**
 * Converts a color from LAB space to XYZ space.
 * \see     http://easyrgb.com
 * \param   LAB    color in L*a*b* space
 * \return  The given color in XYZ space.
 */
vec3 lab2xyz(in vec3 LAB) {
    vec3 XYZ = vec3(0.0);
    XYZ.y = (LAB.x + 16.0) / 116.0;
    XYZ.x = LAB.y / 500.0 + XYZ.y;
    XYZ.z = XYZ.y - LAB.z / 200.0;

    bvec3 mask = greaterThan(pow(XYZ, vec3(3.0)), vec3(0.008856));
    XYZ =   (vec3(mask)      * pow(XYZ, vec3(3.0)))
          + (vec3(not(mask)) * ((XYZ - 16.0 / 116.0) / 7.787));

    return XYZ * colorspace_LabWts_;
}

/**
 * Converts a color from L*a*b* space to L*C*H* space.
 * \see     http://easyrgb.com
 * \param   LAB    color in L*a*b* space
 * \return  The given color in L*C*H* space.
 */
vec3 lab2lch(in vec3 LAB) {
    LAB.y = sqrt(pow(LAB.y, 2.0) + pow(LAB.z, 2.0));
    LAB.z = atan(LAB.z, LAB.y);

    LAB.z = mod(LAB.z, colorspace_two_pi_);

    return LAB;
}

/**
 * Converts a color from L*C*H* space to L*a*b* space.
 * \see     http://easyrgb.com
 * \param   LCH    color in L*C*H* space
 * \return  The given color in L*a*b* space.
 */
vec3 lch2lab(in vec3 LCH) {
    float c = LCH.y;
    LCH.y = cos(LCH.z) * c;
    LCH.z = sin(LCH.z) * c;
    return LCH;
}

/**
 * Converts a color from RGB space to L*a*b* space.
 * \see     http://easyrgb.com
 * \param   RGB    color in RGB space
 * \return  The given color in L*a*b* space.
 */
vec3 rgb2lab(in vec3 RGB) {
    return xyz2lab(rgb2xyz(RGB));
}

/**
 * Converts a color from L*a*b* space to RGB space.
 * \see     http://easyrgb.com
 * \param   LAB    color in L*a*b* space
 * \return  The given color in RGB space.
 */
vec3 lab2rgb(in vec3 LAB) {
    return xyz2rgb(lab2xyz(LAB));
}

/**
 * Converts a color from RGB space to L*C*H* space.
 * \see     http://easyrgb.com
 * \param   RGB    color in RGB space
 * \return  The given color in L*C*H* space.
 */
vec3 rgb2lch(in vec3 RGB) {
    return lab2lch(rgb2lab(RGB));
}

/**
 * Converts a color from L*C*H* space to RGB space.
 * \see     http://easyrgb.com
 * \param   LCH    color in L*C*H* space
 * \return  The given color in RGB space.
 */
vec3 lch2rgb(in vec3 LCH) {
    return lab2rgb(lch2lab(LCH));
}