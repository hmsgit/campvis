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

#ifndef POINTPREDICATE_H__
#define POINTPREDICATE_H__

#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/metaproperty.h"

#include <string>

namespace cgt {
    class Shader;
}

namespace campvis {

    class AbstractPointPredicate : public MetaProperty {
    public:
//         class AbstractColorModulationDecorator {
//         public:
//             AbstractColorModulationDecorator(AbstractPointPredicate* parent)
//                 : _parent(parent)
//             {}
// 
//             virtual ~AbstractColorModulationDecorator() {}
// 
//             virtual const std::string& getHuePart() const = 0;
//             virtual const std::string& getSaturationPart() const = 0;
//             virtual const std::string& getIntensityPart() const = 0;
//         protected:
//             AbstractPointPredicate* _parent;
//         };


        AbstractPointPredicate(const std::string& inputVariable, const std::string& name, const std::string& title);

        virtual ~AbstractPointPredicate();


        /**
         * Returns the GLSL header needed to apply this voxel predicate (i.e. uniforms etc.)
         * \return  The needed GLSL header for this voxel predicate.
         */
        virtual std::string getGlslHeader() const;

        /**
         * Returns the GLSL string to evaluate the predicate.
         * \return  The GLSL string to evaluate the predicate.
         */
        virtual std::string getPredicateEvaluationGlslString() const = 0;
        
        /**
         * Sets up the given shader for this voxel predicate (i.e. sets the uniforms)
         * \param   shader  Shader to set up.
         */
        virtual void setupShader(cgt::Shader* shader) const;

        /**
         * Returns the GLSL uniform name of this predicate's importance value.
         * \return  _importanceUniformName
         */
        const std::string& getImportanceUniformName() const;
        
        /**
         * Returns the GLSL uniform name of this predicate's color value.
         * \return  _colorUniformName
         */
        const std::string& getColorUniformName() const;
        
        /**
         * Returns the GLSL uniform name of this predicate's color value.
         * \return  _colorUniformName
         */
        const std::string& getIntensityHackUniformName() const;

        /**
         * Returns, whether this predicate is enabled.
         * \return  p_enable.getValue()
         */
        bool isEnabled() const;
        
        /**
         * Returns the input variable
         * \return  _inputVariable
         */
        const std::string& getInputVariable() const;

        /// Signal emitted when this predicate's configuration (importance, color, ...) has changed
        sigslot::signal0 s_configurationChanged;
        /// Signal emitted when this predicate's enabled state has changed
        sigslot::signal0 s_enabledChanged;

        BoolProperty p_enable;
        FloatProperty p_importance;
        Vec2Property p_color;

        FloatProperty p_intensityHack;

    protected:
        /**
         * Callback when one of the predicate's properties has changed, issues the 
         * corresponding changed signal to the outside.
         * \param   prop    Property that has changed
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);
        
        std::string _inputVariable;
        const std::string _importanceUniformName;
        const std::string _colorUniformName;
        const std::string _intensityHackUniformName;
    };
    
// ================================================================================================

    class AndCombinedPointPredicate : public AbstractPointPredicate {
    public:
        AndCombinedPointPredicate(const std::string& name, const std::string& title, const std::vector<AbstractPointPredicate*>& predicates);

        virtual ~AndCombinedPointPredicate();


        virtual std::string getGlslHeader() const;
        virtual std::string getPredicateEvaluationGlslString() const;
        virtual void setupShader(cgt::Shader* shader) const;

    protected:
        std::vector<AbstractPointPredicate*> _predicates;
    };

    // ================================================================================================

    class OrCombinedPointPredicate : public AbstractPointPredicate {
    public:
        OrCombinedPointPredicate(const std::string& name, const std::string& title, const std::vector<AbstractPointPredicate*>& predicates);

        virtual ~OrCombinedPointPredicate();


        virtual std::string getGlslHeader() const;
        virtual std::string getPredicateEvaluationGlslString() const;
        virtual void setupShader(cgt::Shader* shader) const;

    protected:
        std::vector<AbstractPointPredicate*> _predicates;
    };

// ================================================================================================

    class RangePointPredicate : public AbstractPointPredicate {
    public:
        RangePointPredicate(const std::string& inputVariable, const std::string& name, const std::string& title);

        virtual ~RangePointPredicate();


        virtual std::string getGlslHeader() const;
        virtual std::string getPredicateEvaluationGlslString() const;
        virtual void setupShader(cgt::Shader* shader) const;

        const std::string& getRangeUniformname() const { return _rangeUniformName; };

        Vec2Property p_range;

    protected:
        const std::string _rangeUniformName;
    };


// ================================================================================================

    class LabelBitPointPredicate : public AbstractPointPredicate {
    public:
        LabelBitPointPredicate(const std::string& inputVariable, const std::string& name, const std::string& title);

        virtual ~LabelBitPointPredicate();


        virtual std::string getGlslHeader() const;
        virtual std::string getPredicateEvaluationGlslString() const;
        virtual void setupShader(cgt::Shader* shader) const;

        IntProperty p_bit;

    protected:
        const std::string _bitUniformName;
    };

}

#endif // POINTPREDICATE_H__
