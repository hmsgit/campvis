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

#ifndef PROPERTYLUAFACTORY_H__
#define PROPERTYLUAFACTORY_H__

#include "cgt/logmanager.h"
#include "cgt/singleton.h"

#include <tbb/atomic.h>
#include <tbb/spin_mutex.h>

#include "scripting/scriptingapi.h"

#include <map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace campvis {
    class AbstractProperty;
    class AbstractPropertyLua;

    /**
     * Factory for creating Lua Property depending on the Property type.
     * Using some template-magic, PropertyLuaFactory is able to register Lua Property during static 
     * initialization in cooperation with the PropertyLuaRegistrar.
     * 
     * \note    PropertyLuaFactory is a thread-safe lazy-instantiated singleton.
     */
    class CAMPVIS_SCRIPTING_API PropertyLuaFactory {
    public:
        /// Typedef for a function pointer to create a PropertyLua if you know exactly its type.
        typedef AbstractPropertyLua* (*PropertyLuaCreateFunctionPointer) (AbstractProperty*);
        /// Typedef for a function pointer to create a PropertyLua using dynamic_casts as a fallback solution
        typedef AbstractPropertyLua* (*FallbackPropertyLuaCreateFunctionPointer) (AbstractProperty*);

        /**
         * Returns a reference to the PipelineFactory singleton.
         * Creates the singleton in a thread-safe fashion, if necessary.
         * \return  *_singleton
         */
        static PropertyLuaFactory& getRef();
    
        /**
         * Deinitializes the Singleton.
         */
        static void deinit();

        /**
         * Registers the the property of type \a type to have luas created with the given function pointers.
         * \note    The template instantiation of PropertyLuaRegistrar takes care of calling this method.
         * \param   type        Property type to register.
         * \param   ptr         Pointer to a function creating the appropriate lua for the property without making type checks (fast).
         * \param   fallbackPtr Pointer to a function creating the appropriate lua for the property only if the type matches - added costs due to the type checks, but also works for unknown types.
         * \param   priority    Priority of \a fallbackPtr compared to other properties' fallbackPtrs to allow semantic ordering.
         * \return 
         */
        size_t registerPropertyLua(const std::type_info& type, PropertyLuaCreateFunctionPointer ptr, FallbackPropertyLuaCreateFunctionPointer fallbackPtr, int priority);

        /**
         * Create a PropertyLua for the given property.
         * Checkes all registered luas, wether they match the type of \a property.
         * \param   property        The property the lua shall handle.
         * \return  The created property lua for the property - nullptr, if there was no matching lua found.
         */
        AbstractPropertyLua* createPropertyLua(AbstractProperty* property);

    private:
        mutable tbb::spin_mutex _mutex;                         ///< Mutex protecting the singleton during initialization
        static tbb::atomic<PropertyLuaFactory*> _singleton;  ///< the singleton object

        /// Typedef for the map associating property types with creator function pointers
        typedef std::map<std::type_index, PropertyLuaCreateFunctionPointer> PropertyLuaMapType;
        /// map associating property types with creator function pointers
        PropertyLuaMapType _propertyWidgetMap;

        /// Map of creator function to use as fallback, if the requested property was not found in the type map.
        std::multimap<int, FallbackPropertyLuaCreateFunctionPointer> _fallbackCreatorMap;
    };


// ================================================================================================

    /**
     * Templated class exploiting the CRTP to allow easy registration of property luas crossing
     * DLL bounds using a single template instantiation.
     * \tparam  PropertyLuaType      Property lua type to register.
     * \tparam  PropertyType    Corresponding property type.
     * \tparam  PRIORITY        Priority for fallback factory instantiation using dynamic_cast type checks.
     */
    template<typename PropertyLuaType, typename PropertyType, int PRIORITY = 0>
    class PropertyLuaRegistrar {
    public:
        /**
         * Static factory method for creating the lua if we know the property type exactly.
         * \param   property        The property the lua shall handle.
         * \return  The newly created property lua.
         */
        static AbstractPropertyLua* create(AbstractProperty* property) {
            cgtAssert(dynamic_cast<PropertyType*>(property) != nullptr, "Incompatible types - this should not happen!");
            return new PropertyLuaType(static_cast<PropertyType*>(property));
        }

        /**
         * Static factory method for creating the lua if we don't know the property type.
         * Performs a dynamic type check to see whether the property type matches.
         * \param   property        The property the lua shall handle.
         * \return  The newly created property lua, may be nullptr in case the types do not match.
         */
        static AbstractPropertyLua* tryCreate(AbstractProperty* property) {
            if (PropertyType* tester = dynamic_cast<PropertyType*>(property))
                return new PropertyLuaType(tester);

            return nullptr;
        }

    private:
        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    // The meat of the static factory registration, call the registerPropertyLua() method and use the 
    // returned size_t to store it in the static member and thus ensure calling during static initialization.
    template<typename PropertyLuaType, typename PropertyType, int PRIORITY>
    const size_t PropertyLuaRegistrar<PropertyLuaType, PropertyType, PRIORITY>::_factoryId 
        = PropertyLuaFactory::getRef().registerPropertyLua(
            typeid(PropertyType), 
            &PropertyLuaRegistrar<PropertyLuaType, PropertyType, PRIORITY>::create, 
            &PropertyLuaRegistrar<PropertyLuaType, PropertyType, PRIORITY>::tryCreate,
            PRIORITY
        );


}

#endif // PROPERTYLUAFACTORY_H__
