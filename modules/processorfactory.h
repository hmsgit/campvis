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
#include <type_traits>
#include <vector>
#include <functional>

#include "core/pipeline/visualizationprocessor.h"
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

        AbstractProcessor* createProcessor(const std::string& id, IVec2Property* viewPortSizeProp = 0) const;

        
        /**
         * Statically registers the processor of type T with construction T() using \a callee as factory method.
         * \note    The template instantiation of ProcessorRegistrar takes care of calling this method.
         * \param   callee  Factory method to call to create an instance of type T
         * \return  The registration index.
         */
        template<typename T>
        size_t registerProcessor(std::function<AbstractProcessor*()> callee) {
            tbb::spin_mutex::scoped_lock lock(_mutex);

            auto it = _processorMap.lower_bound(T::getId());
            if (it == _processorMap.end() || it->first != T::getId()) {
                _processorMap.insert(it, std::make_pair(T::getId(), callee));
            }
            else {
                // do nothing, a double registration may occure due to having the ProcessorRegistrar
                // being referenced in both campvis-application and campvis-modules
            }

            return _processorMap.size();
        }

        /**
         * Statically registers the processor of type T with constructor T(IVec2Property) using \a callee as factory method.
         * \note    The template instantiation of ProcessorRegistrar takes care of calling this method.
         * \param   callee  Factory method to call to create an instance of type T
         * \return  The registration index.
         */
        template<typename T>
        size_t registerProcessor2(std::function<AbstractProcessor*(IVec2Property*)> callee) {
            tbb::spin_mutex::scoped_lock lock(_mutex);

            auto it = _processorMap2.lower_bound(T::getId());
            if (it == _processorMap2.end() || it->first != T::getId()) {
                _processorMap2.insert(it, std::make_pair(T::getId(), callee));
            }
            else {
                // do nothing, a double registration may occure due to having the ProcessorRegistrar
                // being referenced in both campvis-application and campvis-modules
            }

            return _processorMap2.size();
        }

    private:
        mutable tbb::spin_mutex _mutex;
        static tbb::atomic<ProcessorFactory*> _singleton;    ///< the singleton object

        std::map< std::string, std::function<AbstractProcessor*()>> _processorMap;
        std::map< std::string, std::function<AbstractProcessor*(IVec2Property*)>> _processorMap2;
    };


// ================================================================================================

    template<typename T, bool>
    class ProcessorRegistrarSwitch {
    };

    template<typename T>
    class ProcessorRegistrarSwitch<T, false> {
            public:
        /**
         * Static factory method for creating the processor of type T.
         * \return  A newly created processor of type T. Caller has to take ownership of the pointer.
         */
        static AbstractProcessor* create() {
            return new T();
        }

        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    template<typename T>
    const size_t ProcessorRegistrarSwitch<T, false>::_factoryId = ProcessorFactory::getRef().registerProcessor<T>(&ProcessorRegistrarSwitch<T, false>::create);


    template<typename T>
    class ProcessorRegistrarSwitch<T, true> {
    public:
        /**
         * Static factory method for creating the processor of type T.
         * \param   viewPortSizeProp  viewPortSizeProp for the created processor to work on.
         * \return  A newly created processor of type T. Caller has to take ownership of the pointer.
         */
        static AbstractProcessor* create(IVec2Property* viewPortSizeProp) {
            return new T(viewPortSizeProp);
        }

        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    template<typename T>
    const size_t ProcessorRegistrarSwitch<T, true>::_factoryId = ProcessorFactory::getRef().registerProcessor2<T>(&ProcessorRegistrarSwitch<T, true>::create);




    template<typename T>
    class SmartProcessorRegistrar {
        static const size_t _helperField;
    };

    template<typename T>
    const size_t campvis::SmartProcessorRegistrar<T>::_helperField = ProcessorRegistrarSwitch< T, std::is_base_of<VisualizationProcessor, T>::value >::_factoryId;

}

#endif // PROCESSORFACTORY_H__
