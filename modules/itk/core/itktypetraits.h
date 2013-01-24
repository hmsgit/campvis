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

#ifndef ITKTYPETRAITS_H__
#define ITKTYPETRAITS_H__

#include "tgt/tgt_gl.h"
#include "tgt/tgt_math.h"
#include "tgt/vector.h"

#include <itkCovariantVector.h>

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
        typedef itk::CovariantVector< BASETYPE, 2 > ItkElementType;

    };

    template<typename BASETYPE>
    struct ItkTypeTraitsHelperOfBasetypePerChannel<BASETYPE, 3> {
        typedef itk::CovariantVector< BASETYPE, 3 > ItkElementType;

    };

    template<typename BASETYPE>
    struct ItkTypeTraitsHelperOfBasetypePerChannel<BASETYPE, 4> {
        typedef itk::CovariantVector< BASETYPE, 4 > ItkElementType;

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
