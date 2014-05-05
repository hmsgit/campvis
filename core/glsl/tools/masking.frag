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


#ifdef APPLY_MASK
#include "tools/texture2d.frag"
uniform sampler2D _maskImage;   ///< mask image
uniform TextureParameters2D _maskTexParams;
uniform vec4 _maskColor;        ///< mask color

#define MASKING_PROLOG(COORDS) \
    vec4 maskValue = texture(_maskImage, (COORDS) ); \
    if (distance(maskValue, _maskColor) > 0.01) {

#define MASKING_EPILOG \
    } \
    else { \
        discard; \
    }    

#else
#define MASKING_PROLOG
#define MASKING_EPILOG
#endif
