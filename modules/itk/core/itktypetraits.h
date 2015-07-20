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

#ifndef ITKTYPETRAITS_H__
#define ITKTYPETRAITS_H__

#include "cgt/cgt_gl.h"
#include "cgt/cgt_math.h"
#include "cgt/vector.h"

#include <itkVector.h>

#include "core/tools/typetraits.h"

#include <limits>


namespace campvis {

// anonymous namespace for helper traits
// irrelevant for the outside world...
namespace {

    /**
     * Additional Helper Traits for ItkTypeTraits of generic base type varying only in the base type.
     * Template specializations offer the following constants/methods:
     *  - typedef ElementType: Typedef for the type of a single image element
     *  - static BASETYPE getChannel(element, channel): Method for accessing a specific channel of the given image element
     *  - static void getChannel(element, channel, value): Method setting a specific channel of the given image element to \a value
     */
    template<typename BASETYPE, size_t NUMCHANNELS>
    struct ItkTypeTraitsHelperOfBasetypePerChannel {};

    template<typename BASETYPE>
    struct ItkTypeTraitsHelperOfBasetypePerChannel<BASETYPE, 1> {
        typedef BASETYPE ItkElementType;

    };

    template<typename BASETYPE>
    struct ItkTypeTraitsHelperOfBasetypePerChannel<BASETYPE, 2> {
        typedef itk::Vector< BASETYPE, 2 > ItkElementType;

    };

    template<typename BASETYPE>
    struct ItkTypeTraitsHelperOfBasetypePerChannel<BASETYPE, 3> {
        typedef itk::Vector< BASETYPE, 3 > ItkElementType;

    };

    template<typename BASETYPE>
    struct ItkTypeTraitsHelperOfBasetypePerChannel<BASETYPE, 4> {
        typedef itk::Vector< BASETYPE, 4 > ItkElementType;

    };

}

// ================================================================================================
// = Starting with the actual stuff to use ======================================================== 
// ================================================================================================

    /**
     * Additional type traits for ITK image data depending on base type and number of channels.
     *
     * \tparam  BASETYPE    Base type of the image data (type of a single channel of an image element)
     * \tparam  NUMCHANNELS Number of channels of the image data.
     **/
    template<typename BASETYPE, size_t NUMCHANNELS>
    struct ItkTypeTraits {
        /// C++ type of one single image element.
        typedef typename ItkTypeTraitsHelperOfBasetypePerChannel<BASETYPE, NUMCHANNELS>::ItkElementType ItkElementType;

    };

}

#endif // ITKTYPETRAITS_H__
