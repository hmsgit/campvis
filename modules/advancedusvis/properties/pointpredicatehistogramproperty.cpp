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

#include "pointpredicatehistogramproperty.h"

#include <algorithm>
#include <functional>

namespace campvis {

    const std::string PointPredicateHistogramProperty::loggerCat_ = "CAMPVis.modules.advancedusvis.PointPredicateHistogramProperty";


    PointPredicateHistogramProperty::PointPredicateHistogramProperty(const std::string& name, const std::string& title)
        : AbstractProperty(name, title)
    {
        _ignoreSignals = 0;
        std::shared_ptr<PointPredicateHistogram> ptr(new PointPredicateHistogram());
        setHistogramPointer(ptr);
    }

    PointPredicateHistogramProperty::~PointPredicateHistogramProperty() {
    }

    void PointPredicateHistogramProperty::init() {
        AbstractProperty::init();
    }

    void PointPredicateHistogramProperty::deinit() {
        AbstractProperty::deinit();
    }

    void PointPredicateHistogramProperty::lock() {
        AbstractProperty::lock();
        _histogram->lock();
    }

    void PointPredicateHistogramProperty::unlock() {
        _histogram->unlock();
        AbstractProperty::unlock();
    }

    void PointPredicateHistogramProperty::addSharedProperty(AbstractProperty* prop) {
        AbstractProperty::addSharedProperty(prop);

        // We ensure all shared properties to be of correct type the base class method.
        // Hence, static_cast ist safe.
        static_cast<PointPredicateHistogramProperty*>(prop)->setHistogramPointer(this->_histogram);
    }

    PointPredicateHistogram* PointPredicateHistogramProperty::getPredicateHistogram() {
        return _histogram.get();
    }

    const PointPredicateHistogram* PointPredicateHistogramProperty::getPredicateHistogram() const {
        return _histogram.get();
    }

    void PointPredicateHistogramProperty::setHistogramPointer(std::shared_ptr<PointPredicateHistogram> histogram) {
        tgtAssert(histogram != nullptr, "Must not be nullptr!");

        if (_histogram != nullptr) {
            _histogram->s_configurationChanged.disconnect(this);
            _histogram->s_headerChanged.disconnect(this);
        }

        _histogram = histogram;
        _histogram->s_configurationChanged.connect(this, &PointPredicateHistogramProperty::onHistogramConfigurationChanged);
        _histogram->s_headerChanged.connect(this, &PointPredicateHistogramProperty::onHistogramHeaderChanged);

        for (std::set<AbstractProperty*>::iterator it = _sharedProperties.begin(); it != _sharedProperties.end(); ++it) {
            // We ensure all shared properties to be of correct type in the addSharedProperty overload.
            // Hence, static_cast ist safe.
            static_cast<PointPredicateHistogramProperty*>(*it)->setHistogramPointer(histogram);
        }
    }

    void PointPredicateHistogramProperty::onHistogramConfigurationChanged() {
        if (_ignoreSignals == 0)
            s_changed.emitSignal(this);
    }

    void PointPredicateHistogramProperty::onHistogramHeaderChanged() {
        if (_ignoreSignals == 0)
            s_headerChanged.emitSignal();
    }

    std::vector<float> PointPredicateHistogramProperty::getCurrentHistogramDistribution() const {
        std::vector<AbstractPointPredicate*> predicates = _histogram->getPredicates();
        std::vector<float> toReturn(predicates.size(), 0.f);
        for (size_t i = 0; i < predicates.size(); ++i)
            toReturn[i] = predicates[i]->p_importance.getValue();
        return toReturn;
    }

    void PointPredicateHistogramProperty::adjustImportances(std::vector<float> deltas, const std::vector<float>& baseHistogram, int fixedIndex) {
        std::vector<AbstractPointPredicate*> predicates = _histogram->getPredicates();
        tgtAssert(deltas.size() == baseHistogram.size(), "Size of deltas mismatches the size of baseHistogram!");
        tgtAssert(deltas.size() == predicates.size(), "Number of deltas mismatches the number of predicates!");

        // first pass of normalization: ensure sum of deltas = 0:
        float sum = 0.f;
        std::for_each(deltas.begin(), deltas.end(), [&] (float& f) { sum += f; });
        if (sum != 0.f) {
            sum /= deltas.size();
            std::for_each(deltas.begin(), deltas.end(), [&] (float& f) { f -= sum; });
        }

        // compute new importances
        std::vector<float> newImportances(deltas.size(), 0.f);
        for (size_t i = 0; i < baseHistogram.size(); ++i)
            newImportances[i] = baseHistogram[i] + deltas[i];

        // second pass of normalization: ensure all new importances are in [0, 1]
        std::vector<int> adjustableIndices;
        std::vector<int> underflowIndices;
        for (int i = 0; i < static_cast<int>(newImportances.size()); ++i) {
            if (newImportances[i] >= 0.f && i != fixedIndex)
                adjustableIndices.push_back(i);
            else if (newImportances[i] < 0.f)
                underflowIndices.push_back(i);
        }

        // we have to repeatedly do this, as the correction of underflow may underflow other importances...
        do {
            // set each underflowed value to 0 and adjust the other importances by an according fraction.
            std::for_each(underflowIndices.begin(), underflowIndices.end(), [&] (int underflowIndex) {
                if (adjustableIndices.empty()) {
                    LERROR("The vector of adjustable indices is empty, but we need at least one. This should not happen!");
                    return;
                }

                float delta = newImportances[underflowIndex] / adjustableIndices.size();
                std::for_each(adjustableIndices.begin(), adjustableIndices.end(), [&] (int index) { 
                    newImportances[index] += delta; 
                });
                newImportances[underflowIndex] = 0.f;
            });

            // check if we just created new negative importances
            // therefore we remove all indices of now negative values from adjustableIndices and move them to underflowIndices
            // use advanced STL: combine std::resize with std::remove_if to achieve this in O(n) and with few lines of code
            underflowIndices.clear();
            adjustableIndices.resize(
                std::remove_if(adjustableIndices.begin(), adjustableIndices.end(), [&] (int index) -> bool {
                    if (newImportances[index] < 0.f) {
                        underflowIndices.push_back(index);
                        return true;
                    }
                    return false;
                })
                - adjustableIndices.begin());
        } while (! underflowIndices.empty());


        // set new importances:
        ++_ignoreSignals;
        sum = 0.f;
        for (size_t i = 0; i < predicates.size(); ++i) {
            predicates[i]->p_importance.setValue(newImportances[i]);
            sum += newImportances[i];
        }

        if (std::abs(sum - 1.f) > 0.001f)
            LERROR("Sum of importances is not 1 - sth. went wrong!");

        --_ignoreSignals;

        s_changed.emitSignal(this);
    }

}
