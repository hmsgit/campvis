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

#ifndef RAYCASTERFACTORY_H__
#define RAYCASTERFACTORY_H__

#include "cgt/logmanager.h"
#include "cgt/singleton.h"

#include <tbb/atomic.h>
#include <tbb/spin_mutex.h>

#include "core/properties/numericproperty.h"
#include "modules/modulesapi.h"

#include <map>
#include <string>
#include <vector>
#include <functional>

namespace campvis {
    class RaycastingProcessor;

    /**
     * Factory for creating raycasters by their name.
     * Using some template-magic, RaycasterFactory is able to register raycasters during static 
     * initialization in cooperation with the RaycasterRegistrar.
     * 
     * \note    RaycasterFactory is a thread-safe lazy-instantiated singleton.
     */
    class CAMPVIS_MODULES_API RaycasterFactory {
    public:
        /**
         * Returns a reference to the RaycasterFactory singleton.
         * Creates the singleton if necessary
         * \return  *_singleton
         */
        static RaycasterFactory& getRef();
    
        static void deinit();

        std::vector<std::string> getRegisteredRaycasters() const;

        RaycastingProcessor* createRaycaster(const std::string& id, IVec2Property* viewportSizeProp) const;

        /**
         * Statically registers the raycaster of type T using \a callee as factory method.
         * \note    The template instantiation of RaycasterRegistrar takes care of calling this method.
         * \param   callee  Factory method to call to create an instance of type T
         * \return  The registration index.
         */
        template<typename T>
        size_t registerRaycaster(std::function<RaycastingProcessor*(IVec2Property*)> callee) {
            tbb::spin_mutex::scoped_lock lock(_mutex);

            auto it = _raycasterMap.lower_bound(T::getId());
            if (it == _raycasterMap.end() || it->first != T::getId()) {
                _raycasterMap.insert(it, std::make_pair(T::getId(), callee));
            }
            else {
                cgtAssert(false, "Registered two raycasters with the same ID.");
            }
            
            return _raycasterMap.size();
        }
        
    private:
        mutable tbb::spin_mutex _mutex;
        static tbb::atomic<RaycasterFactory*> _singleton;    ///< the singleton object

        std::map< std::string, std::function<RaycastingProcessor*(IVec2Property*)> > _raycasterMap;
    };


// ================================================================================================

    template<typename T>
    class RaycasterRegistrar {
    public:
        /**
         * Static factory method for creating the raycaster of type T.
         * \param   viewportSizeProp  IVec2Property for the created raycaster to work on.
         * \return  A newly created raycaster of type T. Caller has to take ownership of the pointer.
         */
        static RaycastingProcessor* create(IVec2Property* viewportSizeProp) {
            return new T(viewportSizeProp);
        }

    private:
        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    template<typename T>
    const size_t RaycasterRegistrar<T>::_factoryId = RaycasterFactory::getRef().registerRaycaster<T>(&RaycasterRegistrar<T>::create);

}

#endif // RAYCASTERFACTORY_H__
