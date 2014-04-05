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

#include "PredicateVolumeExplorer.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/renderdata.h"

#include "core/classification/simpletransferfunction.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/tools/quadrenderer.h"

#include <algorithm>
#include <functional>

namespace campvis {
    const std::string PredicateVolumeExplorer::loggerCat_ = "CAMPVis.modules.vis.PredicateVolumeExplorer";

    PredicateVolumeExplorer::PredicateVolumeExplorer(IVec2Property* viewportSizeProp)
        : VolumeExplorer(viewportSizeProp, new PointPredicateRaycaster(0))
        , p_inputLabels("InputLabels", "Input Label Image", "", DataNameProperty::READ)
        , p_inputSnr("InputSnr", "Input SNR", "", DataNameProperty::READ)
        , p_inputVesselness("InputVesselness", "Input Vesselness", "", DataNameProperty::READ)
        , p_inputConfidence("InputConfidence", "Input Confidence", "", DataNameProperty::READ)
        , p_histogram("PredicateHistogram", "Point Predicate Histogram")
        , p_pbProperties("PredicateBitsetProperties", "Predicate Bitset Properties", VALID)
        , _predicateEvaluation(_viewportSizeProperty)
        , _bitmaskHandle(nullptr)
        , _mousePressedPointer(nullptr)
    {
        p_inputVolume.addSharedProperty(&_predicateEvaluation.p_inputImage);

        addProperty(&p_inputLabels);
        p_inputLabels.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor()))->p_inputLabels);
        p_inputLabels.addSharedProperty(&_predicateEvaluation.p_inputLabels);

        addProperty(&p_inputSnr);
        p_inputSnr.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor()))->p_inputSnr);
        p_inputSnr.addSharedProperty(&_predicateEvaluation.p_inputSnr);

        addProperty(&p_inputVesselness);
        p_inputVesselness.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor()))->p_inputVesselness);
        p_inputVesselness.addSharedProperty(&_predicateEvaluation.p_inputVesselness);

        addProperty(&p_inputConfidence);
        p_inputConfidence.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor()))->p_inputConfidence);
        p_inputConfidence.addSharedProperty(&_predicateEvaluation.p_inputConfidence);

        addProperty(&p_histogram);
        p_pbProperties.addPropertyCollection(_predicateEvaluation);
        addProperty(&p_pbProperties);

        p_histogram.addSharedProperty(&(static_cast<PointPredicateRaycaster*>(_raycaster.getRaycastingProcessor())->p_predicateHistogram));
        p_histogram.addSharedProperty(&_predicateEvaluation.p_histogram);

        //_raycaster.p_camera.addSharedProperty(&_predicateEvaluation.p_camera);

        _sliceExtractor.p_geometryID.setValue("scribbles");
    }

    PredicateVolumeExplorer::~PredicateVolumeExplorer() {
    }

    void PredicateVolumeExplorer::init() {
        VolumeExplorer::init();
        _predicateEvaluation.init();
        _predicateEvaluation.s_invalidated.connect<PredicateVolumeExplorer>(this, &PredicateVolumeExplorer::onProcessorInvalidated);
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

        if (getInvalidationLevel() & SCRIBBLE_INVALID) {
            FaceGeometry* g = createScribbleGeometry();
            data.addData("scribbles", g);
            validate(SCRIBBLE_INVALID);
        }

        VolumeExplorer::updateResult(data);

        validate(INVALID_RESULT);
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

    void PredicateVolumeExplorer::onEvent(tgt::Event* e) {
        // forward the event to the correspsonding event listeners depending on the mouse position
        if (_bitmaskHandle.getData() != nullptr) {
            const ImageData* id = static_cast<const ImageData*>(_bitmaskHandle.getData());
            if (const GenericImageRepresentationLocal<BitmaskType, 1>* repLocal = id->getRepresentation< GenericImageRepresentationLocal<BitmaskType, 1> >()) {
                const tgt::svec3& imageSize = repLocal->getSize();
                if (typeid(*e) == typeid(tgt::MouseEvent)) {
                    tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);

                    if (me->x() <= p_sliceRenderSize.getValue().x) {
                        float renderTargetRatio = static_cast<float>(p_sliceRenderSize.getValue().x) / static_cast<float>(p_sliceRenderSize.getValue().y);
                        tgt::vec2 ratio = tgt::vec2(static_cast<float>(me->x()), static_cast<float>(me->y() % p_sliceRenderSize.getValue().y)) / tgt::vec2(p_sliceRenderSize.getValue());

                        // TODO: these transformation could probably be integrated into the SliceExtractor much more beautiful, as they are performed there anyway.
                        tgt::vec3 imgSize(repLocal->getSize());

                        tgt::svec3 voxel;
                        if (me->y() <= p_sliceRenderSize.getValue().y) {
                            float sliceRatio = (imgSize.x * repLocal->getParent()->getMappingInformation().getVoxelSize().x) / (imgSize.y * repLocal->getParent()->getMappingInformation().getVoxelSize().y);
                            float ratioRatio = sliceRatio / renderTargetRatio;
                            ratio -= (ratioRatio > 1) ? tgt::vec2(0.f, (1.f - (1.f / ratioRatio)) / 2.f) : tgt::vec2((1.f - ratioRatio) / 2.f, 0.f);
                            ratio *= (ratioRatio > 1) ? tgt::vec2(1.f, ratioRatio) : tgt::vec2(1.f / ratioRatio, 1.f);
                            ratio = tgt::clamp(ratio, 0.f, 1.f);

                            voxel = tgt::clamp(tgt::svec3(static_cast<size_t>(ratio.x * imageSize.x), static_cast<size_t>(ratio.y * imageSize.y), _sliceExtractor.p_zSliceNumber.getValue()), tgt::svec3(0, 0, 0), imageSize);
                        }
                        else if (me->y() <= 2*p_sliceRenderSize.getValue().y) {
                            float sliceRatio = (imgSize.x * repLocal->getParent()->getMappingInformation().getVoxelSize().x) / (imgSize.z * repLocal->getParent()->getMappingInformation().getVoxelSize().z);
                            float ratioRatio = sliceRatio / renderTargetRatio;
                            ratio -= (ratioRatio > 1) ? tgt::vec2(0.f, (1.f - (1.f / ratioRatio)) / 2.f) : tgt::vec2((1.f - ratioRatio) / 2.f, 0.f);
                            ratio *= (ratioRatio > 1) ? tgt::vec2(1.f, ratioRatio) : tgt::vec2(1.f / ratioRatio, 1.f);
                            ratio = tgt::clamp(ratio, 0.f, 1.f);

                            voxel = tgt::clamp(tgt::svec3(static_cast<size_t>(ratio.x * imageSize.x), _sliceExtractor.p_ySliceNumber.getValue(), static_cast<size_t>(ratio.y * imageSize.z)), tgt::svec3(0, 0, 0), imageSize);
                        }
                        else {
                            float sliceRatio = (imgSize.y * repLocal->getParent()->getMappingInformation().getVoxelSize().y) / (imgSize.z * repLocal->getParent()->getMappingInformation().getVoxelSize().z);
                            float ratioRatio = sliceRatio / renderTargetRatio;
                            ratio -= (ratioRatio > 1) ? tgt::vec2(0.f, (1.f - (1.f / ratioRatio)) / 2.f) : tgt::vec2((1.f - ratioRatio) / 2.f, 0.f);
                            ratio *= (ratioRatio > 1) ? tgt::vec2(1.f, ratioRatio) : tgt::vec2(1.f / ratioRatio, 1.f);
                            ratio = tgt::clamp(ratio, 0.f, 1.f);

                            voxel = tgt::clamp(tgt::svec3(_sliceExtractor.p_xSliceNumber.getValue(), static_cast<size_t>(ratio.x * imageSize.y), static_cast<size_t>(ratio.y * imageSize.z)), tgt::svec3(0, 0, 0), imageSize);
                        }

                        if (me->action() == tgt::MouseEvent::PRESSED) {
                            _mousePressedPointer = (me->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT) ? &_yesScribbles : &_noScribbles;
                            if (! me->modifiers() & tgt::Event::SHIFT)
                                _mousePressedPointer->clear();
                            _mousePressedPointer->push_back(voxel);
                            invalidate(INVALID_RESULT | SCRIBBLE_INVALID | SLICES_INVALID);
                            return;
                        }
                        else if (_mousePressedPointer != nullptr && me->action() == tgt::MouseEvent::MOTION) {
                            _mousePressedPointer->push_back(voxel);
                            invalidate(INVALID_RESULT | SCRIBBLE_INVALID | SLICES_INVALID);
                            return;
                        }
                        else if (_mousePressedPointer != nullptr && me->action() == tgt::MouseEvent::RELEASED) {
                            updatePredicateHistogramFromScribbles();
                            _mousePressedPointer = nullptr;
                            return;
                        }
                    }
                }
            }
        }

        VolumeExplorer::onEvent(e);
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

    std::vector<int> PredicateVolumeExplorer::computeBitHistogram(const std::vector<tgt::svec3>& voxels) {
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

    std::vector<tgt::vec3> PredicateVolumeExplorer::createScribbleGeometryVertices(const std::vector<tgt::svec3>& scribble) const {
        const tgt::ivec2& sliceSize = p_sliceRenderSize.getValue();
        std::vector<tgt::vec3> vertices;

        if (_bitmaskHandle.getData() != nullptr) {
            const ImageData* id = static_cast<const ImageData*>(_bitmaskHandle.getData());
            if (const GenericImageRepresentationLocal<BitmaskType, 1>* repLocal = id->getRepresentation< GenericImageRepresentationLocal<BitmaskType, 1> >()) {
                // TODO: these transformation could probably be integrated into the SliceExtractor much more beautiful, as they are performed there anyway.
                tgt::vec3 imgSize(repLocal->getSize());
                float sliceRatioX = (imgSize.y * repLocal->getParent()->getMappingInformation().getVoxelSize().y)
                                  / (imgSize.z * repLocal->getParent()->getMappingInformation().getVoxelSize().z);
                float sliceRatioY = (imgSize.x * repLocal->getParent()->getMappingInformation().getVoxelSize().x)
                                  / (imgSize.z * repLocal->getParent()->getMappingInformation().getVoxelSize().z);
                float sliceRatioZ = (imgSize.x * repLocal->getParent()->getMappingInformation().getVoxelSize().x)
                                  / (imgSize.y * repLocal->getParent()->getMappingInformation().getVoxelSize().y);

                // transform scribbles into viewport coordinates
                for (size_t i = 0; i < scribble.size(); ++i) {
                    const tgt::svec3& v = scribble[i];
                    if (v.x == _sliceExtractor.p_xSliceNumber.getValue()) {
                        //vertices.push_back(tgt::vec3(x.y, v.z, 0.f) * )
                    }
                }
            }
        }
        return vertices;
    }

    FaceGeometry* PredicateVolumeExplorer::createScribbleGeometry() const {
        std::vector<tgt::vec3> vertices;
        std::vector<tgt::vec4> colors;

        for (size_t i = 0; i < _yesScribbles.size(); ++i) {
            vertices.push_back(tgt::vec3(_yesScribbles[i]));
            colors.push_back(tgt::vec4(.2f, .8f, 0.f, 1.f));
        }
        for (size_t i = 0; i < _noScribbles.size(); ++i) {
            vertices.push_back(tgt::vec3(_noScribbles[i]));
            colors.push_back(tgt::vec4(.85f, .2f, 0.f, 1.f));
        }

        return new FaceGeometry(vertices, std::vector<tgt::vec3>(), colors);
    }

}

