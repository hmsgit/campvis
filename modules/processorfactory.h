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

#ifndef PROCESSORFACTORY_H__
#define PROCESSORFACTORY_H__

#include "cgt/logmanager.h"
#include "cgt/singleton.h"

#include <tbb/atomic.h>
#include <tbb/spin_mutex.h>

#include "modules/modulesapi.h"

#include <map>
#include <string>
#include <vector>
#include <functional>
#include "core/properties/numericproperty.h"

namespace campvis {
    class AbstractProcessor;
    class DataContainer;

    /**
     * Factory for creating processors by their name.
     * Using some template-magic, ProcessorFactory is able to register processors during static 
     * initialization in cooperation with the ProcessorRegistrar.
     * 
     * \note    ProcessorFactory is a thread-safe lazy-instantiated singleton.
     */
    class CAMPVIS_MODULES_API ProcessorFactory {
    public:
        /**
         * Returns a reference to the ProcessorFactory singleton.
         * Creates the singleton if necessary
         * \return  *_singleton
         */
        static ProcessorFactory& getRef();
    
        static void deinit();
        
        std::vector<std::string> getRegisteredProcessors() const;

        AbstractProcessor* createProcessor(const std::string& id, IVec2Property* viewPortSizeProp) const;

        /**
         * Statically registers the processor of type T using \a callee as factory method.
         * \note    The template instantiation of ProcessorRegistrar takes care of calling this method.
         * \param   callee  Factory method to call to create an instance of type T
         * \return  The registration index.
         */
        template<typename T>
        size_t registerProcessor(std::function<AbstractProcessor*(IVec2Property*)> callee) {
            tbb::spin_mutex::scoped_lock lock(_mutex);

            auto it = _processorMap.lower_bound(T::getId());
            if (it == _processorMap.end() || it->first != T::getId()) {
                _processorMap.insert(it, std::make_pair(T::getId(), callee));
            }
            else {
                cgtAssert(false, "Registered two processors with the same ID.");
            }

            return _processorMap.size();
        }
        
    private:
        mutable tbb::spin_mutex _mutex;
        static tbb::atomic<ProcessorFactory*> _singleton;    ///< the singleton object

        std::map< std::string, std::function<AbstractProcessor*(IVec2Property*)>> _processorMap;


    };


// ================================================================================================

    template<typename T>
    class ProcessorRegistrar {
    public:
        /**
         * Static factory method for creating the processor of type T.
         * \param   args  DataContainer for the created processor to work on.
         * \return  A newly created processor of type T. Caller has to take ownership of the pointer.
         */
        //static AbstractProcessor* create() {
        //    return new T();
        //}


        static AbstractProcessor* create(IVec2Property* viewPortSizeProp) {
            return new T(viewPortSizeProp);
        }

    private:
        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    template<typename T>
    const size_t ProcessorRegistrar<T>::_factoryId = ProcessorFactory::getRef().registerProcessor<T>(&ProcessorRegistrar<T>::create);

}

#endif // PROCESSORFACTORY_H__
