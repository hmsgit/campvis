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

#ifndef INTPROPERTYLUA_H__
#define INTPROPERTYLUA_H__

#include "abstractpropertylua.h"
#include "propertyluafactory.h"
#include "core/properties/numericproperty.h"
#include "core/tools/stringutils.h"

namespace campvis {
    /**
     * Lua generator for a IntProperty
     */
    class IntPropertyLua : public AbstractPropertyLua {
    public:
        /**
         * Creates a new IntPropertyLua for the property \a property.
         * \param   property        The property the Lua shall handle
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr.
         */
        IntPropertyLua(IntProperty* property);

        /**
         * Destructor
         */
        virtual ~IntPropertyLua();

        std::string getLuaScript(const std::string& propNamePrefix, const std::string& luaProc);
    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyLuaRegistrar<IntPropertyLua, IntProperty>;

// ================================================================================================

    namespace {
        template<size_t SIZE>
        struct IVecPropertyLuaTraits {};

        template<>
        struct IVecPropertyLuaTraits<2> {
            typedef IVec2Property PropertyType;
            typedef cgt::ivec2 BaseType;
        };

        template<>
        struct IVecPropertyLuaTraits<3> {
            typedef IVec3Property PropertyType;
            typedef cgt::ivec3 BaseType;
        };

        template<>
        struct IVecPropertyLuaTraits<4> {
            typedef IVec4Property PropertyType;
            typedef cgt::ivec4 BaseType;
        };
    }

// ================================================================================================

    /**
     * Generic base class for IVec property Luas.
     */
    template<size_t SIZE>
    class IVecPropertyLua : public AbstractPropertyLua {
    public:
        enum { size = SIZE };
        typedef typename IVecPropertyLuaTraits<SIZE>::PropertyType PropertyType;

        /**
         * Creates a new IVecPropertyLua for the property \a property.
         * \param   property    The property the Lua shall handle
         */
        IVecPropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr);

        /**
         * Destructor
         */
        virtual ~IVecPropertyLua();

        virtual std::string getLuaScript(const std::string& propNamePrefix, const std::string& luaProc);
    };

// ================================================================================================

    template<size_t SIZE>
    campvis::IVecPropertyLua<SIZE>::IVecPropertyLua(PropertyType* property, DataContainer* dataContainer)
        : AbstractPropertyLua(property)
    {
    }

    template<size_t SIZE>
    campvis::IVecPropertyLua<SIZE>::~IVecPropertyLua() {
    }

    template<size_t SIZE>
    std::string IVecPropertyLua<SIZE>::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
        std::string ret = "-- NOT IMPLEMENTED IVecProperty";
        return ret;
    }

// ================================================================================================

    class IVec2PropertyLua : public IVecPropertyLua<2> {
    public:
        IVec2PropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr)
            : IVecPropertyLua<2>(property, dataContainer)
        {
        }

        std::string IVec2PropertyLua::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
            cgt::ivec2 value = static_cast<IVec2Property*>(_property)->getValue();
            std::string ret = "";
            ret += luaProc;
            ret += "getNestedProperty(\"" + propNamePrefix + _property->getName() + "\"):setValue(cgt.ivec2(" 
                + StringUtils::toString(value.x) +", " + StringUtils::toString(value.y) + "))";
            return ret;
        }
    }; 

// ================================================================================================
    
    class IVec3PropertyLua : public IVecPropertyLua<3> {
    public:
        IVec3PropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr)
            : IVecPropertyLua<3>(property, dataContainer)
        {
        }

        std::string IVec3PropertyLua::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
            cgt::ivec3 value = static_cast<IVec3Property*>(_property)->getValue();
            std::string ret = "";
            ret += luaProc;
            ret += "getNestedProperty(\"" + propNamePrefix + _property->getName() + "\"):setValue(cgt.ivec3(" 
                + StringUtils::toString(value.x) +", " + StringUtils::toString(value.y) +", "
                + StringUtils::toString(value.z) + "))";
            return ret;
        }
    }; 

// ================================================================================================

    class IVec4PropertyLua : public IVecPropertyLua<4> {
    public:
        IVec4PropertyLua(PropertyType* property, DataContainer* dataContainer = nullptr)
            : IVecPropertyLua<4>(property, dataContainer)
        {
        }

        std::string IVec4PropertyLua::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
            cgt::ivec4 value = static_cast<IVec4Property*>(_property)->getValue();
            std::string ret = "";
            ret += luaProc;
            ret += "getNestedProperty(\"" + propNamePrefix + _property->getName() + "\"):setValue(cgt.ivec4(" 
                + StringUtils::toString(value.x) +", " + StringUtils::toString(value.y) +", "
                + StringUtils::toString(value.z) +", " + StringUtils::toString(value.w) + "))";
            return ret;
        }
    }; 

// ================================================================================================

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyLuaRegistrar<IVec2PropertyLua, typename IVecPropertyLuaTraits<2>::PropertyType, 5>;
    template class PropertyLuaRegistrar<IVec3PropertyLua, typename IVecPropertyLuaTraits<3>::PropertyType, 5>;
    template class PropertyLuaRegistrar<IVec4PropertyLua, typename IVecPropertyLuaTraits<4>::PropertyType, 5>;

}
#endif // INTPROPERTYLUA_H__
