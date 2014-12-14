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
     * Lua generator for a FloatProperty
     */
    class FloatPropertyLua : public AbstractPropertyLua {
    public:
        /**
         * Creates a new FloatPropertyLua for the property \a property.
         * \param   property    The property the Lua shall handle
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr.
         */
        FloatPropertyLua(FloatProperty* property);

        /**
         * Destructor
         */
        virtual ~FloatPropertyLua();

        std::string getLuaScript(const std::string& propNamePrefix, const std::string& luaProc);
        
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
            typedef cgt::vec2 BaseType;
        };

        template<>
        struct VecPropertyLuaTraits<3> {
            typedef Vec3Property PropertyType;
            typedef cgt::vec3 BaseType;
        };

        template<>
        struct VecPropertyLuaTraits<4> {
            typedef Vec4Property PropertyType;
            typedef cgt::vec4 BaseType;
        };
    }

// ================================================================================================

    /**
     * Generic base class for Vec property Luas.
     */
    template<size_t SIZE>
    class VecPropertyLua : public AbstractPropertyLua {
    public:
        enum { size = SIZE };
        typedef typename VecPropertyLuaTraits<SIZE>::PropertyType PropertyType;

        /**
         * Creates a new VecPropertyLua for the property \a property.
         * \param   property    The property the Lua shall handle
         */
        VecPropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr);

        /**
         * Destructor
         */
        virtual ~VecPropertyLua();

        virtual std::string getLuaScript(const std::string& propNamePrefix, const std::string& luaProc);

    };

// ================================================================================================

    template<size_t SIZE>
    VecPropertyLua<SIZE>::VecPropertyLua(PropertyType* property, DataContainer* dataContainer)
        : AbstractPropertyLua(property)
    {
    }

    template<size_t SIZE>
    VecPropertyLua<SIZE>::~VecPropertyLua() {
        PropertyType* property = static_cast<PropertyType*>(_property);
    }

    template<size_t SIZE>
    std::string VecPropertyLua<SIZE>::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
        std::string ret = "-- NOT IMPLEMENTED VecProperty";
        return ret;
    }
// ================================================================================================

    class Vec2PropertyLua : public VecPropertyLua<2> {
    public:
        Vec2PropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr)
            : VecPropertyLua<2>(property, dataContainer)
        {
        }

        std::string Vec2PropertyLua::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
            cgt::vec2 value = static_cast<Vec2Property*>(_property)->getValue();
            std::string ret = "";
            ret += luaProc;
            ret += "getNestedProperty(\"" + propNamePrefix + _property->getName() + "\"):setValue(cgt.vec2(" 
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
        std::string Vec3PropertyLua::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
            cgt::vec3 value = static_cast<Vec3Property*>(_property)->getValue();
            std::string ret = "";
            ret += luaProc;
            ret += "getNestedProperty(\"" + propNamePrefix + _property->getName() + "\"):setValue(cgt.vec3(" 
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

        std::string Vec4PropertyLua::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
            cgt::vec4 value = static_cast<Vec4Property*>(_property)->getValue();
            std::string ret = "";
            ret += luaProc;
            ret += "getNestedProperty(\"" + propNamePrefix + _property->getName() + "\"):setValue(cgt.vec4(" 
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
