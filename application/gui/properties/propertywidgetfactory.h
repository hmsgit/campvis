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

#ifndef PROPERTYWIDGETFACTORY_H__
#define PROPERTYWIDGETFACTORY_H__

namespace campvis {
    class AbstractPropertyWidget;
    class AbstractProperty;
    class DataContainer;

    /**
     * Factory class offering the static method createWidget to create property widgets for a given
     * property (depending on its type).
     */
    class PropertyWidgetFactory {
    public:
        /**
         * Creates the corresponding property widget for the given property \a property.
         * \note    The callee has to take the ownership of the returned pointer.
         * \param   property    Property to generate the widget for.
         * \param   dc          Pointer to the DataContainer the property shall use - defaults to 0.
         * \return  A new property widget for the given property (depending on its type).
         */
        static AbstractPropertyWidget* createWidget(AbstractProperty* property, DataContainer* dc = 0);
    };
}
#endif // PROPERTYWIDGETFACTORY_H__
