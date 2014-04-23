// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef IMAGEREPRESENTATIONCONVERTER_H__
#define IMAGEREPRESENTATIONCONVERTER_H__

#include "tgt/logmanager.h"
#include "tgt/singleton.h"

#include <tbb/atomic.h>
#include <tbb/spin_mutex.h>

#include "core/coreapi.h"
#include "core/datastructures/abstractimagerepresentation.h"

#include <map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

class QWidget;

namespace campvis {
    /**
     * Factory for creating PropertyWidgets depending on the Property type.
     * Using some template-magic, ImageRepresentationConverter is able to register PropertyWidgets during static 
     * initialization in cooperation with the ConversionFunctionRegistrar.
     * 
     * \note    ImageRepresentationConverter is a thread-safe lazy-instantiated singleton.
     */
    class CAMPVIS_CORE_API ImageRepresentationConverter {
    public:
        /// Typedef for a function pointer to convert between image representations.
        typedef const AbstractImageRepresentation* (*ConversionFunctionPointer) (const AbstractImageRepresentation*);

        /**
         * Returns a reference to the PipelineFactory singleton.
         * Creates the singleton in a thread-safe fashion, if necessary.
         * \return  *_singleton
         */
        static ImageRepresentationConverter& getRef();
    
        /**
         * Deinitializes the Singleton.
         */
        static void deinit();

        /**
         * Registers the the property of type \a type to have widgets created with the given function pointers.
         * \note    The template instantiation of ConversionFunctionRegistrar takes care of calling this method.
         * \param   type        Property type to register.
         * \param   funcPtr     Pointer to a conversion function.
         * \return 
         */
        size_t registerConversionFunction(const std::type_info& type, ConversionFunctionPointer funcPtr);


        template<typename T>
        const T* tryConvertFrom(const AbstractImageRepresentation* source) {
            auto itPair = _conversionFunctionMap.equal_range(std::type_index(typeid(T)));

            // try conversion with all registered convertes for the given target type
            for (auto it = itPair.first; it != itPair.second; ++it) {
                const T* tester = static_cast<const T*>(it->second(source));
                if (tester != nullptr)
                    return tester;
            }

            return nullptr;
        };

    private:
        mutable tbb::spin_mutex _mutex;                                 ///< Mutex protecting the singleton during initialization
        static tbb::atomic<ImageRepresentationConverter*> _singleton;   ///< the singleton object

        /// Typedef for the map associating property types with conversion function pointers
        typedef std::multimap<std::type_index, ConversionFunctionPointer> ConversionFunctionMapType;
        /// map associating property types with creator function pointers
        ConversionFunctionMapType _conversionFunctionMap;
    };


// ================================================================================================

    /**
     * Templated class exploiting the CRTP to allow easy registration of property widgets crossing
     * DLL bounds using a single template instantiation.
     * \tparam  WidgetType      Property widget type to register.
     */
    template<typename ConversionTarget, typename ConverterClass>
    class ConversionFunctionRegistrar {
    public:
        static const AbstractImageRepresentation* tryConvertFrom(const AbstractImageRepresentation* source) {
            return ConverterClass::tryConvertFrom(source);
        }

    private:
        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    // The meat of the static factory registration, call the registerConversionFunction() method and use the 
    // returned size_t to store it in the static member and thus ensure calling during static initialization.
    template<typename ConversionTarget, typename ConverterClass>
    const size_t ConversionFunctionRegistrar<ConversionTarget, ConverterClass>::_factoryId 
        = ImageRepresentationConverter::getRef().registerConversionFunction(
            typeid(ConversionTarget), 
            &ConversionFunctionRegistrar<ConversionTarget, ConverterClass>::tryConvertFrom
        );

}

#endif // IMAGEREPRESENTATIONCONVERTER_H__
