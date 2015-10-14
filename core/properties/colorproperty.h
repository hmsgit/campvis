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


#ifndef COLORPROPERTY_H__
#define COLORPROPERTY_H__


# include "core/properties/floatingpointproperty.h"

namespace campvis {
    template <typename T>
    class _ColorProperty : public FloatingPointProperty<T> {
    public:
                /**
             * Creates a new FloatingPointProperty.
             * \param name      Property name
             * \param title     Property title (e.g. used for GUI)
             * \param value     Initial value of the property
             * \param minValue  Minimum value for this property
             * \param maxValue  Maximum value for this property
             * \param stepValue Step value for this property
             * \param decimals  Number of significant decimal places
             */
            _ColorProperty(
                const std::string& name,
                const std::string& title,
                const T& value,
                const T& minValue,
                const T& maxValue,
                const T& stepValue = T(0.01f));

            static const std::string loggerCat_;
    };

    template<typename T>
    const std::string campvis::_ColorProperty<T>::loggerCat_ = "CAMPVis.core.properties.ColorProperty";

    typedef _ColorProperty<cgt::vec4> ColorProperty;

    template<typename T>
    _ColorProperty<T>::_ColorProperty(
        const std::string& name,
        const std::string& title,
        const T& value,
        const T& minValue,
        const T& maxValue,
        const T& stepValue) 
        : Vec4Property(name, title, value, minValue, maxValue, stepValue)
    {

    }
}


#endif // COLORPROPERTY_H__

