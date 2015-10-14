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

#ifndef ABSTRACTDATA_H__
#define ABSTRACTDATA_H__

#include <memory>
#include <tbb/atomic.h>
#include "cgt/bounds.h"

#include "core/coreapi.h"
#include "core/datastructures/scopedtypeddata.h" // not directly needed here but by many classes including AbstractData 

namespace campvis {

    /**
     * Interface class for data which can compute their bounds in world coordinates.
     */
    class CAMPVIS_CORE_API IHasWorldBounds {
    public:
        /// Default empty constructor
        IHasWorldBounds() {};
        /// Virtual Destructor
        virtual ~IHasWorldBounds() {};

        /**
         * Returns the data extent in world coordinates.
         * \return  The data extent in world coordinates.
         */
        virtual cgt::Bounds getWorldBounds() const = 0;
    };

// ================================================================================================

    class DataHandle;

    /**
     * Abstract base class for data handled by a DataHandle and stored in a DataContainer.
     */
    class CAMPVIS_CORE_API AbstractData {
    friend class DataHandle;

    public:
        /**
         * Constructor, simply calles ReferenceCounted ctor.
         */
        AbstractData();

        /**
         * Virtual destructor
         */
        virtual ~AbstractData();


        /**
         * Prototype - clone method, some people call this virtual constructor...
         * \return  A copy of this object.
         */
        virtual AbstractData* clone() const = 0;

        /**
         * Returns the local memory footprint of the data in bytes.
         * \return  Number of bytes occupied in local memory by the data.
         */
        virtual size_t getLocalMemoryFootprint() const = 0;

        /**
         * Returns the video memory footprint of the data in bytes.
         * \return  Number of bytes occupied in video memory by the data.
         */
        virtual size_t getVideoMemoryFootprint() const = 0;

        /**
         * Returns a human readable representation of the data type that can be used for the GUI.
         * \return  A string with the type name to be used for the GUI.
         */
        virtual std::string getTypeAsString() const = 0;

    private:
        /// This weak_ptr points to the shared_ptr owning group of this object, if existant.
        /// Should be only accessed by DataHandle (therefore the friendship) in order to avoid
        /// multiple owning groups for the same object.
        std::weak_ptr<AbstractData> _weakPtr;
    };

}

#endif // ABSTRACTDATA_H__
