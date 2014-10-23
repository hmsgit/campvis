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

#include "predicatevolumeexplorer.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "core/classification/simpletransferfunction.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/tools/quadrenderer.h"

#include <algorithm>
#include <functional>

namespace campvis {
    const std::string PredicateVolumeExplorer::loggerCat_ = "CAMPVis.modules.vis.PredicateVolumeExplorer";

    PredicateVolumeExplorer::PredicateVolumeExplorer(IVec2Property* viewportSizeProp)
        : VolumeExplorer(viewportSizeProp, new SliceExtractor(0), new PointPredicateRaycaster(0))
        , p_inputLabels("InputLabels", "Input Label Image", "", DataNameProperty::READ)
        , p_inputSnr("InputSnr", "Input SNR", "", DataNameProperty::READ)
        , p_inputVesselness("InputVesselness", "Input Vesselness", "", DataNameProperty::READ)
        , p_inputConfidence("InputConfidence", "Input Confidence", "", DataNameProperty::READ)
        , p_histogram("PredicateHistogram", "Point Predicate Histogram")
        , p_pbProperties("PredicateBitsetProperties", "Predicate Bitset Properties")
        , _predicateEvaluation(_viewportSizeProperty)
        , _bitmaskHandle(nullptr)
    {
        p_inputVolume.addSharedProperty(&_predicateEvaluation.p_inputImage);

        addProperty(p_inputLabels);
        p_inputLabels.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor()))->p_inputLabels);
        p_inputLabels.addSharedProperty(&_predicateEvaluation.p_inputLabels);

        addProperty(p_inputSnr);
        p_inputSnr.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor()))->p_inputSnr);
        p_inputSnr.addSharedProperty(&_predicateEvaluation.p_inputSnr);

        addProperty(p_inputVesselness);
        p_inputVesselness.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor()))->p_inputVesselness);
        p_inputVesselness.addSharedProperty(&_predicateEvaluation.p_inputVesselness);

        addProperty(p_inputConfidence);
        p_inputConfidence.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor()))->p_inputConfidence);
        p_inputConfidence.addSharedProperty(&_predicateEvaluation.p_inputConfidence);

        addProperty(p_histogram);
        p_pbProperties.addPropertyCollection(_predicateEvaluation);
        addProperty(p_pbProperties, VALID);

        p_histogram.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor())->p_predicateHistogram));
        p_histogram.addSharedProperty(&_predicateEvaluation.p_histogram);
    }

    PredicateVolumeExplorer::~PredicateVolumeExplorer() {
    }

    void PredicateVolumeExplorer::init() {
        VolumeExplorer::init();
        _predicateEvaluation.init();
        _predicateEvaluation.s_invalidated.connect<PredicateVolumeExplorer>(this, &PredicateVolumeExplorer::onProcessorInvalidated);

        p_enableScribbling.setValue(true);
    }

    void PredicateVolumeExplorer::deinit() {
        _predicateEvaluation.s_invalidated.disconnect(this);
        _predicateEvaluation.deinit();
        _bitmaskHandle = DataHandle(0);
        VolumeExplorer::deinit();
    }

    void PredicateVolumeExplorer::updateResult(DataContainer& data) {
        // launch sub-renderers if necessary
        if (getInvalidationLevel() & BITSET_INVALID) {
            _predicateEvaluation.process(data);
            
            ImageRepresentationLocal::ScopedRepresentation repLocal(data, _predicateEvaluation.p_outputImage.getValue());
            _bitmaskHandle = repLocal.getDataHandle();
            validate(BITSET_INVALID);
        }

        VolumeExplorer::updateResult(data);
    }

    void PredicateVolumeExplorer::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &p_inputVolume || prop == &p_histogram) {
            invalidate(BITSET_INVALID);
        }

        VolumeExplorer::onPropertyChanged(prop);
    }

    void PredicateVolumeExplorer::onProcessorInvalidated(AbstractProcessor* processor) {
        // make sure to only invalidate ourself if the invalidation is not triggered by us
        // => the _locked state is a trustworthy source for this information :)
        if (! isLocked()) {
            if (processor == &_predicateEvaluation) {
                invalidate(BITSET_INVALID);
            }

            invalidate(AbstractProcessor::INVALID_RESULT);
        }

        VolumeExplorer::onProcessorInvalidated(processor);
    }

    void PredicateVolumeExplorer::onEvent(cgt::Event* e) {
        bool doHistogramUpdateFromScribbles = false;

        // intercept the finishing of a scribble paint, so that we can trigger the predicate histogram update.
        if (typeid(*e) == typeid(cgt::MouseEvent)) {
            cgt::MouseEvent* me = static_cast<cgt::MouseEvent*>(e);
            if (_scribblePointer != nullptr && me->action() == cgt::MouseEvent::RELEASED) {
                doHistogramUpdateFromScribbles = true;
            }
        }

        VolumeExplorer::onEvent(e);

        if (doHistogramUpdateFromScribbles)
            updatePredicateHistogramFromScribbles();
    }

    void PredicateVolumeExplorer::updatePredicateHistogramFromScribbles() {
        std::vector<int> yesHistogram = computeBitHistogram(_yesScribbles);
        std::vector<int> noHistogram = computeBitHistogram(_noScribbles);
        std::vector<float> differences(yesHistogram.size());

        // compute differences, their sum and their absolute sum
        int diffSumAbs = 0;
        std::transform(
            yesHistogram.begin(), yesHistogram.end(), noHistogram.begin(), differences.begin(), 
            [&] (int a, int b) -> int { diffSumAbs += std::abs(a - b); return a - b; });

        if (diffSumAbs == 0)
            return;

        // compute 
        float quantile = .25f / static_cast<float>(diffSumAbs);
        std::for_each(differences.begin(), differences.end(), [&] (float& f) { f *= quantile; });
        p_histogram.adjustImportances(differences, p_histogram.getCurrentHistogramDistribution());

        invalidate(INVALID_RESULT);
    }

    std::vector<int> PredicateVolumeExplorer::computeBitHistogram(const std::vector<cgt::vec3>& voxels) {
        std::vector<int> toReturn = std::vector<int>(p_histogram.getPredicateHistogram()->getPredicates().size(), 0);

        if (_bitmaskHandle.getData() != nullptr) {
            const ImageData* id = static_cast<const ImageData*>(_bitmaskHandle.getData());
            if (const GenericImageRepresentationLocal<BitmaskType, 1>* repLocal = id->getRepresentation< GenericImageRepresentationLocal<BitmaskType, 1> >()) {
                // traverse all voxels and for each bitmask check each bit whether its set
                for (size_t i = 0; i < voxels.size(); ++i) {
                    BitmaskType bm = repLocal->getElement(voxels[i]);

                    for (size_t bit = 0; bit < toReturn.size(); ++bit) {
                        // if bit is set, increment bit counter
                        if (bm & 1 << bit)
                            ++(toReturn[bit]);
                    }
                }
            }
        }

        return toReturn;
    }

}

