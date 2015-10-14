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

#ifndef PROCESSORFACTORY_H__
#define PROCESSORFACTORY_H__

#include "cgt/logmanager.h"

#include <tbb/atomic.h>
#include <tbb/spin_mutex.h>

#include "core/coreapi.h"

#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <typeindex>
#include <vector>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/raycastingprocessor.h"

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
    class CAMPVIS_CORE_API ProcessorFactory {
    public:
        /**
         * Returns a reference to the ProcessorFactory singleton.
         * Creates the singleton if necessary
         * \return  *_singleton
         */
        static ProcessorFactory& getRef();
    
        static void deinit();
        
        /**
         * Returns the list of all registered Processors.
         * \return  A std::vector of the string IDs of all registered processors.
         */
        std::vector<std::string> getRegisteredProcessors() const;

        /**
         * Returns the list of all registered raycasting processors (all registered processors 
         * inheriting from RaycastingProcessor).
         * \return  A std::vector of the string IDs of all registered raycasting processors.
         */
        std::vector<std::string> getRegisteredRaycastingProcessors() const;

        /**
         * Factory method to create a processor from the given string ID.
         * \param   id                  String ID of the processor to create.
         * \param   viewPortSizeProp    Pointer to the viewport size property that the created VisualizationProcessor should use. If the created processor is no VisualizationProcessor, then this argument is ignored. Defaults to nullptr.
         * \return  Pointer to the newly created processor, may be nullptr. Caller has to take ownership of the returned pointer.
         */
        AbstractProcessor* createProcessor(const std::string& id, IVec2Property* viewPortSizeProp = nullptr) const;

        
        /**
         * Statically registers the processor of type T with construction T() using \a callee as factory method.
         * \note    The template instantiation of ProcessorRegistrar takes care of calling this method.
         * \param   callee  Factory method to call to create an instance of type T
         * \return  The registration index.
         */
        template<typename T>
        size_t registerProcessorWithDefaultConstructor(std::function<AbstractProcessor*()> callee) {
            tbb::spin_mutex::scoped_lock lock(_mutex);

            // add to list of raycasting processors if needed
            if (std::is_base_of<RaycastingProcessor, T>::value)
                _raycastingProcessors.push_back(T::getId());

            auto it = _processorTypeMap.find(T::getId());
            if (it != _processorTypeMap.end()) {
                // check, whether the type is the same, then a double registration is okay since it
                // may occur due to having the ProcessorRegistrar being referenced in both 
                // campvis-application and campvis-modules.
                cgtAssert(std::type_index(typeid(T)) == it->second, "Tried to register two different processor types with the same ID.");
            }
            else {
                _processorTypeMap.insert(it, std::make_pair(T::getId(), std::type_index(typeid(T))));
            }

            _processorMapDefault[T::getId()] = callee;
            return _processorMapDefault.size();
        }

        /**
         * Statically registers the processor of type T with constructor T(IVec2Property) using \a callee as factory method.
         * \note    The template instantiation of ProcessorRegistrar takes care of calling this method.
         * \param   callee  Factory method to call to create an instance of type T
         * \return  The registration index.
         */
        template<typename T>
        size_t registerProcessorWithIVec2PropParam(std::function<AbstractProcessor*(IVec2Property*)> callee) {
            tbb::spin_mutex::scoped_lock lock(_mutex);

            // add to list of raycasting processors if needed
            if (std::is_base_of<RaycastingProcessor, T>::value)
                _raycastingProcessors.push_back(T::getId());

            auto it = _processorTypeMap.find(T::getId());
            if (it != _processorTypeMap.end()) {
                // check, whether the type is the same, then a double registration is okay since it
                // may occur due to having the ProcessorRegistrar being referenced in both 
                // campvis-application and campvis-modules.
                cgtAssert(std::type_index(typeid(T)) == it->second, "Tried to register two different processor types with the same ID.");
            }
            else {
                _processorTypeMap.insert(it, std::make_pair(T::getId(), std::type_index(typeid(T))));
            }

            _processorMapWithIVec2Param[T::getId()] = callee;
            return _processorMapWithIVec2Param.size();
        }

    private:
        mutable tbb::spin_mutex _mutex;
        static tbb::atomic<ProcessorFactory*> _singleton;    ///< the singleton object

        std::map< std::string, std::type_index> _processorTypeMap;
        std::map< std::string, std::function<AbstractProcessor*()> > _processorMapDefault;
        std::map< std::string, std::function<AbstractProcessor*(IVec2Property*)> > _processorMapWithIVec2Param;

        std::vector<std::string> _raycastingProcessors;
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
    const size_t ProcessorRegistrarSwitch<T, false>::_factoryId = ProcessorFactory::getRef().registerProcessorWithDefaultConstructor<T>(&ProcessorRegistrarSwitch<T, false>::create);


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
    const size_t ProcessorRegistrarSwitch<T, true>::_factoryId = ProcessorFactory::getRef().registerProcessorWithIVec2PropParam<T>(&ProcessorRegistrarSwitch<T, true>::create);

    
    /**
     * Smart processor registrar that uses type traits to deduce the base type of the processor to
     * register and forwards its registration to the corresponding ProcessorRegistrarSwitch<T, bool>.
     */
    template<typename T>
    class SmartProcessorRegistrar {
        static const size_t _helperField;
    };

    template<typename T>
    const size_t campvis::SmartProcessorRegistrar<T>::_helperField = ProcessorRegistrarSwitch< T, std::is_base_of<VisualizationProcessor, T>::value >::_factoryId;

}

#endif // PROCESSORFACTORY_H__
