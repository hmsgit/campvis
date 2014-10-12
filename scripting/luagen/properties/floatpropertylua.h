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

#ifndef FLOATPROPERTYLUA_H__
#define FLOATPROPERTYLUA_H__

#include "abstractpropertylua.h"
#include "propertyluafactory.h"
#include "core/properties/floatingpointproperty.h"
#include "core/tools/stringutils.h"

namespace campvis {
    /**
     * Lua for a FloatProperty
     */
    class FloatPropertyLua : public AbstractPropertyLua {
    public:
        /**
         * Creates a new FloatPropertyLua for the property \a property.
         * \param   property    The property the Lua shall handle
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr.
         * \param   parent      Parent Qt Lua
         */
        FloatPropertyLua(FloatProperty* property, DataContainer* dataContainer);

        /**
         * Destructor
         */
        virtual ~FloatPropertyLua();

        std::string getLuaScript();
        
    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyLuaRegistrar<FloatPropertyLua, FloatProperty>;

// ================================================================================================

    namespace {
        template<size_t SIZE>
        struct VecPropertyLuaTraits {};

        template<>
        struct VecPropertyLuaTraits<2> {
            typedef Vec2Property PropertyType;
            typedef tgt::vec2 BaseType;
        };

        template<>
        struct VecPropertyLuaTraits<3> {
            typedef Vec3Property PropertyType;
            typedef tgt::vec3 BaseType;
        };

        template<>
        struct VecPropertyLuaTraits<4> {
            typedef Vec4Property PropertyType;
            typedef tgt::vec4 BaseType;
        };
    }

// ================================================================================================

    /**
     * Generic base class for Vec property Luas.
     * Unfortunately Q_OBJECT and templates do not fit together, so we an additional level of 
     * indirection helps as usual...
     */
    template<size_t SIZE>
    class VecPropertyLua : public AbstractPropertyLua {
    public:
        enum { size = SIZE };
        typedef typename VecPropertyLuaTraits<SIZE>::PropertyType PropertyType;

        /**
         * Creates a new VecPropertyLua for the property \a property.
         * \param   property    The property the Lua shall handle
         * \param   parent      Parent Qt Lua
         */
        VecPropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr);

        /**
         * Destructor
         */
        virtual ~VecPropertyLua();

        virtual std::string getLuaScript();

    };

// ================================================================================================

    template<size_t SIZE>
    campvis::VecPropertyLua<SIZE>::VecPropertyLua(PropertyType* property, DataContainer* dataContainer)
        : AbstractPropertyLua(property, true, dataContainer)
    {
    }

    template<size_t SIZE>
    campvis::VecPropertyLua<SIZE>::~VecPropertyLua() {
        PropertyType* property = static_cast<PropertyType*>(_property);
    }

    template<size_t SIZE>
    std::string campvis::VecPropertyLua<SIZE>::getLuaScript() {
        std::string ret = "-- NOT IMPLEMENTED VecProperty";
        //ret += "getProperty(" + _property->getName() + "):setValue(" + StringUtils::toString<bool>( static_cast<BoolProperty*>(_property)->getValue() ) + ")";
        return ret;
    }
// ================================================================================================

    class Vec2PropertyLua : public VecPropertyLua<2> {
    public:
        Vec2PropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr)
            : VecPropertyLua<2>(property, dataContainer)
        {
        }

        std::string campvis::Vec2PropertyLua::getLuaScript() {
            tgt::vec2 value = static_cast<Vec2Property*>(_property)->getValue();
            std::string ret = "";
            ret += "getProperty(\"" + _property->getName() + "\"):setValue(tgt.vec2(" 
                + StringUtils::toString(value.x) +", " + StringUtils::toString(value.y) + "))";
            return ret;
        }
    }; 

// ================================================================================================
    
    class Vec3PropertyLua : public VecPropertyLua<3> {
    public:
        Vec3PropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr)
            : VecPropertyLua<3>(property, dataContainer)
        {
        }
        std::string campvis::Vec3PropertyLua::getLuaScript() {
            tgt::vec3 value = static_cast<Vec3Property*>(_property)->getValue();
            std::string ret = "";
            ret += "getProperty(\"" + _property->getName() + "\"):setValue(tgt.vec3(" 
                + StringUtils::toString(value.x) +", " + StringUtils::toString(value.y) 
                +", " + StringUtils::toString(value.z) + "))";
            return ret;
        }
    }; 

// ================================================================================================

    class Vec4PropertyLua : public VecPropertyLua<4> {
    public:
        Vec4PropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr)
            : VecPropertyLua<4>(property, dataContainer)
        {
        }

        std::string campvis::Vec4PropertyLua::getLuaScript() {
            tgt::vec4 value = static_cast<Vec4Property*>(_property)->getValue();
            std::string ret = "";
            ret += "getProperty(\"" + _property->getName() + "\"):setValue(tgt.vec4(" 
                + StringUtils::toString(value.x) +", " + StringUtils::toString(value.y) +", "
                + StringUtils::toString(value.z) +", " + StringUtils::toString(value.w) + "))";
            return ret;
        }
    };

// ================================================================================================

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyLuaRegistrar<Vec2PropertyLua, typename VecPropertyLuaTraits<2>::PropertyType, 5>;
    template class PropertyLuaRegistrar<Vec3PropertyLua, typename VecPropertyLuaTraits<3>::PropertyType, 5>;
    template class PropertyLuaRegistrar<Vec4PropertyLua, typename VecPropertyLuaTraits<4>::PropertyType, 5>;

}

#endif // FLOATPROPERTYLUA_H__
