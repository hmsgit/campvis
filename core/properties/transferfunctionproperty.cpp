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

#include "transferfunctionproperty.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"

#include <tbb/tbb.h>

namespace campvis {

    const std::string TransferFunctionProperty::loggerCat_ = "CAMPVis.core.datastructures.TransferFunctionProperty";

    TransferFunctionProperty::TransferFunctionProperty(const std::string& name, const std::string& title, AbstractTransferFunction* tf)
        : AbstractProperty(name, title)
        , _transferFunction(tf)
        , _imageHandle(0)
        , _autoFitWindowToData(true)
    {
        cgtAssert(tf != 0, "Assigned transfer function must not be 0.");
        
        tf->s_changed.connect(this, &TransferFunctionProperty::onTFChanged);
        tf->s_intensityDomainChanged.connect(this, &TransferFunctionProperty::onTfIntensityDomainChanged);

        _intensityHistogram = 0;
        _dirtyHistogram = false;
    }

    TransferFunctionProperty::~TransferFunctionProperty() {
        delete _transferFunction;
        delete _intensityHistogram;
    }

    AbstractTransferFunction* TransferFunctionProperty::getTF() {
        return _transferFunction;
    }

    void TransferFunctionProperty::onTFChanged() {
        s_changed.emitSignal(this);
    }

    void TransferFunctionProperty::deinit() {
        _transferFunction->s_changed.disconnect(this);
        _transferFunction->s_intensityDomainChanged.disconnect(this);
        _transferFunction->deinit();
        _imageHandle = DataHandle(0);
    }

    void TransferFunctionProperty::replaceTF(AbstractTransferFunction* tf) {
        cgtAssert(tf != 0, "Transfer function must not be 0.");
        s_BeforeTFReplace.triggerSignal(_transferFunction); // use trigger to force blocking signal handling in same thread

        if (_transferFunction != 0) {
            _transferFunction->s_changed.disconnect(this);
            _transferFunction->s_intensityDomainChanged.disconnect(this);
            _transferFunction->deinit();
        }
        delete _transferFunction;


        _transferFunction = tf;
        if (_transferFunction != 0) {
            _transferFunction->s_changed.connect(this, &TransferFunctionProperty::onTFChanged);
            _transferFunction->s_intensityDomainChanged.connect(this, &TransferFunctionProperty::onTfIntensityDomainChanged);
        }

        s_AfterTFReplace.emitSignal(_transferFunction);
    }

    void TransferFunctionProperty::addSharedProperty(AbstractProperty* prop) {
        cgtAssert(false, "Sharing of TF properties not supported!");
    }

    campvis::DataHandle TransferFunctionProperty::getImageHandle() const {
        return _imageHandle;
    }

    void TransferFunctionProperty::setImageHandle(DataHandle imageHandle) {
        cgtAssert(
            imageHandle.getData() == 0 || dynamic_cast<const ImageData*>(imageHandle.getData()) != 0, 
            "The data in the image handle must either be 0 or point to a valid ImageData object!");

        if (_autoFitWindowToData && imageHandle.getData() != 0) {
            if (const ImageData* id = dynamic_cast<const ImageData*>(imageHandle.getData())) {
            	const ImageRepresentationLocal* localRep = id->getRepresentation<ImageRepresentationLocal>();
                if (localRep != 0) {
                    const Interval<float>& ii = localRep->getNormalizedIntensityRange();
                    _transferFunction->setIntensityDomain(cgt::vec2(ii.getLeft(), ii.getRight()));
                }
            }
        }

        _imageHandle = imageHandle;
        _dirtyHistogram = true;
        s_imageHandleChanged.emitSignal();
    }

    void TransferFunctionProperty::setAutoFitWindowToData(bool newValue) {
        _autoFitWindowToData = newValue;
        s_autoFitWindowToDataChanged.emitSignal();
    }

    bool TransferFunctionProperty::getAutoFitWindowToData() const {
        return _autoFitWindowToData;
    }

    void TransferFunctionProperty::computeIntensityHistogram() const {
        IntensityHistogramType* newHistogram = 0;

        // create new histogram according to the current intensity domain
        ImageRepresentationLocal::ScopedRepresentation repLocal(_imageHandle);
        if (repLocal != 0) {
            float mins = _transferFunction->getIntensityDomain().x;
            float maxs = _transferFunction->getIntensityDomain().y;
            size_t numBuckets = std::min(WeaklyTypedPointer::numBytes(repLocal->getWeaklyTypedPointer()._baseType) << 8, static_cast<size_t>(512));
            newHistogram = new IntensityHistogramType(&mins, &maxs, &numBuckets);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, repLocal->getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    float value = repLocal->getElementNormalized(i, 0);
                    newHistogram->addSample(&value);
                }
            });
        }

        // atomically replace old histogram with the new one and delete the old one.
        IntensityHistogramType* oldHistogram = _intensityHistogram.fetch_and_store(newHistogram);
        delete oldHistogram;
        _dirtyHistogram = false;
    }

    const TransferFunctionProperty::IntensityHistogramType* TransferFunctionProperty::getIntensityHistogram() const {
        if (_dirtyHistogram) {
            computeIntensityHistogram();
        }

        return _intensityHistogram;
    }

    void TransferFunctionProperty::onTfIntensityDomainChanged() {
        _dirtyHistogram = true;
    }

}
