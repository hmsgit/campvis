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

#include "pointpredicate.h"

#include "tgt/shadermanager.h"
#include "core/tools/stringutils.h"


namespace campvis {


    AbstractPointPredicate::AbstractPointPredicate(const std::string& inputVariable, const std::string& name, const std::string& title) 
        : MetaProperty(name, title)
        , p_enable("Enable", "Enable Predicate", true)
        , p_importance("Importance", "Importance", 1.f, 0.f, 5.f, .1f, 1)
        , p_color("Color", "Predicate-Specific Color", tgt::vec2(0.f), tgt::vec2(0.f), tgt::vec2(1.f), tgt::vec2(.01f, .1f), tgt::ivec2(2, 1))
        , p_intensityHack("IntensityHack", "Intensity Hack", 0.f, 0.f, 1.f, 1)
        , _inputVariable(inputVariable)
        , _importanceUniformName("_vpImportance" + _name)
        , _colorUniformName("_vpColor" + _name)
        , _intensityHackUniformName("_vpIntensityHack" + _name)
    {
        tgtAssert(inputVariable.find_first_of(" \t\r\n") == std::string::npos, "Input variable must not contain whitespace!");
        tgtAssert(name.find_first_of(" \t\r\n") == std::string::npos, "Predicate name must not contain whitespace!");

        p_enable.setVisible(false);
        p_importance.setVisible(false);
        p_color.setVisible(false);
        p_intensityHack.setVisible(false);

        addProperty(p_enable);
        addProperty(p_importance);
        addProperty(p_color);
        addProperty(p_intensityHack);
    }

    AbstractPointPredicate::~AbstractPointPredicate() {
    }

    std::string AbstractPointPredicate::getGlslHeader() const {
        std::string toReturn;
        toReturn += "uniform float " + _importanceUniformName + ";\n";
        toReturn += "uniform float " + _intensityHackUniformName + ";\n";
        toReturn += "uniform vec2 " + _colorUniformName + ";\n";

        return toReturn;
    }

    void AbstractPointPredicate::setupShader(tgt::Shader* shader) const {
        shader->setUniform(_importanceUniformName, p_importance.getValue());
        shader->setUniform(_colorUniformName, p_color.getValue());
        shader->setUniform(_intensityHackUniformName, p_intensityHack.getValue());
    }

    const std::string& AbstractPointPredicate::getImportanceUniformName() const {
        return _importanceUniformName;
    }

    const std::string& AbstractPointPredicate::getColorUniformName() const {
        return _colorUniformName;
    }

    const std::string& AbstractPointPredicate::getIntensityHackUniformName() const {
        return _intensityHackUniformName;
    }

    bool AbstractPointPredicate::isEnabled() const {
        return p_enable.getValue();
    }

    const std::string& AbstractPointPredicate::getInputVariable() const {
        return _inputVariable;
    }

    void AbstractPointPredicate::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &p_enable) {
            if (p_enable.getValue()) {
                for (size_t i = 0; i < _properties.size(); ++i) {
                    _properties[i]->setVisible(true);
                }
            }
            else {
                for (size_t i = 0; i < _properties.size(); ++i) {
                    if (_properties[i] != &p_enable)
                        _properties[i]->setVisible(false);
                }
            }

            s_enabledChanged.emitSignal();
        }
        else {
            s_configurationChanged.emitSignal();
        }

        MetaProperty::onPropertyChanged(prop);
    }

// ================================================================================================
    
    AndCombinedPointPredicate::AndCombinedPointPredicate(const std::string& name, const std::string& title, const std::vector<AbstractPointPredicate*>& predicates) 
        : AbstractPointPredicate("", name, title)
        , _predicates(predicates)
    {
        for (size_t i = 0; i < _predicates.size(); ++i) {
            addProperty(*_predicates[i]);
        }
    }

    AndCombinedPointPredicate::~AndCombinedPointPredicate() {
        for (size_t i = 0; i < _predicates.size(); ++i)
            delete _predicates[i];
    }

    std::string AndCombinedPointPredicate::getGlslHeader() const {
        std::string toReturn = AbstractPointPredicate::getGlslHeader();
        for (size_t i = 0; i < _predicates.size(); ++i)
            toReturn += _predicates[i]->getGlslHeader();

        return toReturn;
    }

    std::string AndCombinedPointPredicate::getPredicateEvaluationGlslString() const {
        std::string toReturn = "(" + _predicates.front()->getPredicateEvaluationGlslString();
        for (size_t i = 1; i < _predicates.size(); ++i)
            toReturn += " && " + _predicates[i]->getPredicateEvaluationGlslString();
        toReturn += ")";

        return toReturn;
    }

    void AndCombinedPointPredicate::setupShader(tgt::Shader* shader) const {
        for (size_t i = 0; i < _predicates.size(); ++i)
            _predicates[i]->setupShader(shader);

        AbstractPointPredicate::setupShader(shader);
    }

    // ================================================================================================

    OrCombinedPointPredicate::OrCombinedPointPredicate(const std::string& name, const std::string& title, const std::vector<AbstractPointPredicate*>& predicates) 
        : AbstractPointPredicate("", name, title)
        , _predicates(predicates)
    {
        for (size_t i = 0; i < _predicates.size(); ++i) {
            addProperty(*_predicates[i]);
        }
    }

    OrCombinedPointPredicate::~OrCombinedPointPredicate() {
        for (size_t i = 0; i < _predicates.size(); ++i)
            delete _predicates[i];
    }

    std::string OrCombinedPointPredicate::getGlslHeader() const {
        std::string toReturn = AbstractPointPredicate::getGlslHeader();
        for (size_t i = 0; i < _predicates.size(); ++i)
            toReturn += _predicates[i]->getGlslHeader();

        return toReturn;
    }

    std::string OrCombinedPointPredicate::getPredicateEvaluationGlslString() const {
        std::string toReturn = "(" + _predicates.front()->getPredicateEvaluationGlslString();
        for (size_t i = 1; i < _predicates.size(); ++i)
            toReturn += " || " + _predicates[i]->getPredicateEvaluationGlslString();
        toReturn += ")";

        return toReturn;
    }

    void OrCombinedPointPredicate::setupShader(tgt::Shader* shader) const {
        for (size_t i = 0; i < _predicates.size(); ++i)
            _predicates[i]->setupShader(shader);

        AbstractPointPredicate::setupShader(shader);
    }


// ================================================================================================

    RangePointPredicate::RangePointPredicate(const std::string& inputVariable, const std::string& name, const std::string& title)
        : AbstractPointPredicate(inputVariable, name, title)
        , p_range("Range", "Range", tgt::vec2(0.f, 1.f), tgt::vec2(0.f), tgt::vec2(1.f), tgt::vec2(.01f), tgt::ivec2(2))
        , _rangeUniformName("_vpRange" + _name)
    {
        addProperty(p_range);
    }

    RangePointPredicate::~RangePointPredicate() {

    }

    std::string RangePointPredicate::getGlslHeader() const {
        std::string toReturn = AbstractPointPredicate::getGlslHeader();
        toReturn += "uniform vec2 " + _rangeUniformName + ";\n";

        return toReturn;
    }

    std::string RangePointPredicate::getPredicateEvaluationGlslString() const {
        return "(" + _inputVariable + " >= " + _rangeUniformName + ".x && " + _inputVariable + " <= " + _rangeUniformName + ".y)";
    }

    void RangePointPredicate::setupShader(tgt::Shader* shader) const {
        AbstractPointPredicate::setupShader(shader);
        shader->setUniform(_rangeUniformName, p_range.getValue());
    }

// ================================================================================================

    LabelBitPointPredicate::LabelBitPointPredicate(const std::string& inputVariable, const std::string& name, const std::string& title)
        : AbstractPointPredicate(inputVariable, name, title)
        , p_bit("Bit", "Bit in Label Image", 0, 0, 16)
        , _bitUniformName("_vpBit" + _name)
    {
        addProperty(p_bit);
    }

    LabelBitPointPredicate::~LabelBitPointPredicate() {

    }

    std::string LabelBitPointPredicate::getGlslHeader() const {
        std::string toReturn = AbstractPointPredicate::getGlslHeader();
        toReturn += "uniform int " + _bitUniformName + ";\n";

        return toReturn;
    }

    std::string LabelBitPointPredicate::getPredicateEvaluationGlslString() const {
        return "(bitfieldExtract(" + _inputVariable + ", " + _bitUniformName + ", 1) != 0U)";
    }

    void LabelBitPointPredicate::setupShader(tgt::Shader* shader) const {
        AbstractPointPredicate::setupShader(shader);
        shader->setUniform(_bitUniformName, p_bit.getValue());
    }

}