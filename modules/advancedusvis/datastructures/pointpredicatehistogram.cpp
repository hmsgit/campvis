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

#include "pointpredicatehistogram.h"

#include "core/tools/stringutils.h"

namespace campvis {

    const std::string PointPredicateHistogram::loggerCat_ = "CAMPVis.modules.advancedusvis.PointPredicateHistogram";;


    PointPredicateHistogram::PointPredicateHistogram() {

    }

    PointPredicateHistogram::~PointPredicateHistogram() {

    }

    void PointPredicateHistogram::addPredicate(AbstractPointPredicate* predicateToAdd) {
        predicateToAdd->p_importance.setValue(_predicates.empty() ? 1.f : 0.f);
        predicateToAdd->s_configurationChanged.connect(this, &PointPredicateHistogram::onPredicateConfigurationChanged);
        predicateToAdd->s_enabledChanged.connect(this, &PointPredicateHistogram::onPredicateEnabledChanged);

        _predicates.push_back(predicateToAdd);
        s_headerChanged();
    }

    void PointPredicateHistogram::removePredicate(size_t i) {
        tgtAssert(i < _predicates.size(), "Index out of bounds!");

        _predicates[i]->s_configurationChanged.disconnect(this);
        _predicates[i]->s_enabledChanged.disconnect(this);
        _predicates[i]->deinit();
        delete _predicates[i];

        _predicates.erase(_predicates.begin() + i);
        s_headerChanged();
    }

    void PointPredicateHistogram::resetPredicates() {
        float imp = 1.f / _predicates.size();
        for (size_t i = 0; i < _predicates.size(); ++i) {
            _predicates[i]->p_importance.setValue(imp);
            _predicates[i]->p_color.setValue(tgt::vec2(0.f));
        }

        s_configurationChanged();
    }

    std::vector<AbstractPointPredicate*>& PointPredicateHistogram::getPredicates() {
        return _predicates;
    }

    std::string PointPredicateHistogram::getGlslHeader() const {
        std::string toReturn;
        for (size_t i = 0; i < _predicates.size(); ++i) {
            toReturn += _predicates[i]->getGlslHeader();
        }

        toReturn +=
            "vec4 performPredicateBasedShading(in float intensity, in float gradientMagnitude, in float gradientAngle, float viewAngle, in float snr, in float vesselness, in float confidence, in uint label) {\n"
            "    float impCount = 0.0;\n"
            "    float hueCount = 0.0;\n"
            "    vec4 impSum = vec4(0.0, 0.0, 0.0, 1.0);\n";

        for (size_t i = 0; i < _predicates.size(); ++i) {
            if (_predicates[i]->isEnabled()) {
                toReturn += 
                    "    if " + _predicates[i]->getPredicateEvaluationGlslString() + " {\n"
                    "        float imp = pow(" + _predicates[i]->getImportanceUniformName() + " * " + StringUtils::toString(_predicates.size()) + ", 2.0);\n"
                    "        impSum.x += " + _predicates[i]->getColorUniformName() + ".x * " + _predicates[i]->getColorUniformName() + ".y * imp;\n"
                    "        impSum.y += " + _predicates[i]->getColorUniformName() + ".y * imp;\n"
                    "        impSum.z += " + _predicates[i]->getIntensityHackUniformName() + ";\n"
                    "        impSum.a += 1.0;\n"
                    "        hueCount += " + _predicates[i]->getColorUniformName() + ".y * imp;\n"
                    "        impCount += imp;\n"
                    "    }\n";
            }
        }

        toReturn +=
            "    if (impCount > 0.0) {\n"
            "        impSum.x /= hueCount;\n"
            "        impSum.y /= impCount;\n"
            "        impSum.a = impCount / impSum.a;\n"
            "    }\n"
            "    else\n"
            "        impSum = vec4(0.0);\n"
            "    return impSum;\n"
            "}\n\n";

        toReturn +=
            "uint computePredicateBitfield(in float intensity, in float gradientMagnitude, in float gradientAngle, float viewAngle, in float snr, in float vesselness, in float confidence, in uint label) {\n"
            "    uint toReturn = 0U;\n";

        for (size_t i = 0; i < _predicates.size(); ++i) {
            toReturn += 
                "    if " + _predicates[i]->getPredicateEvaluationGlslString() + " {\n"
                "        toReturn = bitfieldInsert(toReturn, 0xFFFFFFFF, " + StringUtils::toString(i) + ", 1);\n"
                "    }\n";
        }

        toReturn +=
            "    return toReturn;\n"
            "}\n\n";

        return toReturn;
    }

    void PointPredicateHistogram::setupRenderShader(tgt::Shader* shader) const {
        for (size_t i = 0; i < _predicates.size(); ++i) {
            //if (_predicates[i]->isEnabled())
                _predicates[i]->setupShader(shader);
        }
    }

    void PointPredicateHistogram::onPredicateConfigurationChanged() {
        s_configurationChanged();
    }

    void PointPredicateHistogram::onPredicateEnabledChanged() {
        s_headerChanged();
    }

    void PointPredicateHistogram::lock() {
        for (size_t i = 0; i < _predicates.size(); ++i)
            _predicates[i]->lockAllProperties();
    }

    void PointPredicateHistogram::unlock() {
        for (size_t i = 0; i < _predicates.size(); ++i)
            _predicates[i]->unlockAllProperties();
    }

}