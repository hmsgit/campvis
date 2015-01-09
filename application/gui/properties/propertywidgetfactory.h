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

#ifndef PROPERTYWIDGETFACTORY_H__
#define PROPERTYWIDGETFACTORY_H__

#include "cgt/logmanager.h"
#include "cgt/singleton.h"

#include <tbb/atomic.h>
#include <tbb/spin_mutex.h>

#include <map>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

class QWidget;

namespace campvis {
    class AbstractProperty;
    class AbstractPropertyWidget;
    class DataContainer;

    /**
     * Factory for creating PropertyWidgets depending on the Property type.
     * Using some template-magic, PropertyWidgetFactory is able to register PropertyWidgets during static 
     * initialization in cooperation with the PropertyWidgetRegistrar.
     * 
     * \note    PropertyWidgetFactory is a thread-safe lazy-instantiated singleton.
     */
    class PropertyWidgetFactory {
    public:
        /// Typedef for a function pointer to create a PropertyWidget if you know exactly its type.
        typedef AbstractPropertyWidget* (*PropertyWidgetCreateFunctionPointer) (AbstractProperty*, DataContainer*, QWidget*);
        /// Typedef for a function pointer to create a PropertyWidget using dynamic_casts as a fallback solution
        typedef AbstractPropertyWidget* (*FallbackPropertyWidgetCreateFunctionPointer) (AbstractProperty*, DataContainer*, QWidget*);

        /**
         * Returns a reference to the PipelineFactory singleton.
         * Creates the singleton in a thread-safe fashion, if necessary.
         * \return  *_singleton
         */
        static PropertyWidgetFactory& getRef();
    
        /**
         * Deinitializes the Singleton.
         */
        static void deinit();

        /**
         * Registers the the property of type \a type to have widgets created with the given function pointers.
         * \note    The template instantiation of PropertyWidgetRegistrar takes care of calling this method.
         * \param   type        Property type to register.
         * \param   ptr         Pointer to a function creating the appropriate widget for the property without making type checks (fast).
         * \param   fallbackPtr Pointer to a function creating the appropriate widget for the property only if the type matches - added costs due to the type checks, but also works for unknown types.
         * \param   priority    Priority of \a fallbackPtr compared to other properties' fallbackPtrs to allow semantic ordering.
         * \return 
         */
        size_t registerPropertyWidget(const std::type_info& type, PropertyWidgetCreateFunctionPointer ptr, FallbackPropertyWidgetCreateFunctionPointer fallbackPtr, int priority);

        /**
         * Create a PropertyWidget for the given property.
         * Checkes all registered widgets, wether they match the type of \a property.
         * \param   property        The property the widget shall handle.
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr. However, some derived classed might need a valid pointer here!
         * \param   parent          Parent Qt widget, defaults to nullptr.
         * \return  The created property widget for the property - nullptr, if there was no matching widget found.
         */
        AbstractPropertyWidget* createWidget(AbstractProperty* property, DataContainer* dataContainer = nullptr, QWidget* parent = nullptr);

    private:
        mutable tbb::spin_mutex _mutex;                         ///< Mutex protecting the singleton during initialization
        static tbb::atomic<PropertyWidgetFactory*> _singleton;  ///< the singleton object

        /// Typedef for the map associating property types with creator function pointers
        typedef std::map<std::type_index, PropertyWidgetCreateFunctionPointer> PropertyWidgetMapType;
        /// map associating property types with creator function pointers
        PropertyWidgetMapType _propertyWidgetMap;

        /// Map of creator function to use as fallback, if the requested property was not found in the type map.
        std::multimap<int, FallbackPropertyWidgetCreateFunctionPointer> _fallbackCreatorMap;
    };


// ================================================================================================

    /**
     * Templated class exploiting the CRTP to allow easy registration of property widgets crossing
     * DLL bounds using a single template instantiation.
     * \tparam  WidgetType      Property widget type to register.
     * \tparam  PropertyType    Corresponding property type.
     * \tparam  PRIORITY        Priority for fallback factory instantiation using dynamic_cast type checks.
     */
    template<typename WidgetType, typename PropertyType, int PRIORITY = 0>
    class PropertyWidgetRegistrar {
    public:
        /**
         * Static factory method for creating the widget if we know the property type exactly.
         * \param   property        The property the widget shall handle.
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr. However, some derived classed might need a valid pointer here!
         * \param   parent          Parent Qt widget, defaults to nullptr.
         * \return  The newly created property widget.
         */
        static AbstractPropertyWidget* create(AbstractProperty* property, DataContainer* dc = nullptr, QWidget* parent = nullptr) {
            cgtAssert(dynamic_cast<PropertyType*>(property) != nullptr, "Incompatible types - this should not happen!");
            return new WidgetType(static_cast<PropertyType*>(property), dc, parent);
        }

        /**
         * Static factory method for creating the widget if we don't know the property type.
         * Performs a dynamic type check to see whether the property type matches.
         * \param   property        The property the widget shall handle.
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr. However, some derived classed might need a valid pointer here!
         * \param   parent          Parent Qt widget, defaults to nullptr.
         * \return  The newly created property widget, may be nullptr in case the types do not match.
         */
        static AbstractPropertyWidget* tryCreate(AbstractProperty* property, DataContainer* dc = nullptr, QWidget* parent = nullptr) {
            if (PropertyType* tester = dynamic_cast<PropertyType*>(property))
                return new WidgetType(tester, dc, parent);

            return nullptr;
        }

    private:
        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    // The meat of the static factory registration, call the registerPropertyWidget() method and use the 
    // returned size_t to store it in the static member and thus ensure calling during static initialization.
    template<typename WidgetType, typename PropertyType, int PRIORITY>
    const size_t PropertyWidgetRegistrar<WidgetType, PropertyType, PRIORITY>::_factoryId 
        = PropertyWidgetFactory::getRef().registerPropertyWidget(
            typeid(PropertyType), 
            &PropertyWidgetRegistrar<WidgetType, PropertyType, PRIORITY>::create, 
            &PropertyWidgetRegistrar<WidgetType, PropertyType, PRIORITY>::tryCreate,
            PRIORITY
        );


}
#endif // PROPERTYWIDGETFACTORY_H__
