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

#include "lhhistogram.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include <tbb/tbb.h>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/tools/concurrenthistogram.h"

namespace campvis {
    class LHGenerator {
    public:
        LHGenerator(const ImageRepresentationLocal* intensities, const GenericImageRepresentationLocal<float, 4>* gradients, ImageRepresentationLocal* fl, ImageRepresentationLocal* fh, float epsilon)
            : _intensities(intensities)
            , _gradients(gradients)
            , _fl(fl)
            , _fh(fh)
            , _epsilon(epsilon)
        {
            cgtAssert(_intensities->getDimensionality() == _gradients->getDimensionality(), "Dimensionality of intensities volumes must match!");
            cgtAssert(_intensities->getSize() == _gradients->getSize(), "Size of intensities volumes must match!");
        }

        void operator() (const tbb::blocked_range<size_t>& range) const {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                cgt::svec3 pos = _intensities->getParent()->indexToPosition(i);

                const cgt::vec4& gradient = _gradients->getElement(i);
                float fl = _intensities->getElementNormalized(i, 0);
                float fh = fl;

                if (gradient.w > 0) {
                    float forwardIntensity = integrateHeun(cgt::vec3(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z)), gradient);
                    float backwardIntensity = integrateHeun(cgt::vec3(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z)), gradient * -1.f);

                    fh = std::max(forwardIntensity, backwardIntensity);
                    fl = std::min(forwardIntensity, backwardIntensity);
                }

                _fl->setElementNormalized(i, 0, fl);
                _fh->setElementNormalized(i, 0, fh);
            }
        }

    protected:
        cgt::vec4 getGradientLinear(const cgt::vec3& position) const {
            cgt::vec4 result;
            result.x = _gradients->getElementNormalizedLinear(position, 0);
            result.y = _gradients->getElementNormalizedLinear(position, 1);
            result.z = _gradients->getElementNormalizedLinear(position, 2);
            result.w = _gradients->getElementNormalizedLinear(position, 3);
            return result;
        }

        float integrateHeun(cgt::vec3 position, const cgt::vec4& direction) const {
            cgt::vec4 gradient1 = direction;
            cgt::vec3 stepSize(.25f);
            cgt::vec3 size(_intensities->getSize());
            size_t numSteps = 0;

            while (abs(gradient1.w) < _epsilon) {
                cgt::vec4 gradient2 = getGradientLinear(position + cgt::normalize(gradient1.xyz()) * stepSize/2.f);
                position += cgt::normalize((gradient1 + gradient2).xyz()) * stepSize;
                gradient1 = getGradientLinear(position);
                ++numSteps;

                if (numSteps > 128 || cgt::hor(cgt::lessThan(position, cgt::vec3::zero)) || cgt::hor(cgt::greaterThan(position, size)))
                    break;
            }

            return _intensities->getElementNormalizedLinear(position, 0);;
        }

        const ImageRepresentationLocal* _intensities;
        const GenericImageRepresentationLocal<float, 4>* _gradients;
        ImageRepresentationLocal* _fl;
        ImageRepresentationLocal* _fh;
        float _epsilon;
    };

// ================================================================================================

    class LHHistogramGenerator {
    public:
        LHHistogramGenerator(const ImageRepresentationLocal* fl, const ImageRepresentationLocal* fh, ConcurrentGenericHistogramND<float, 2>* histogram)
            : _fl(fl)
            , _fh(fh)
            , _histogram(histogram)
        {
            cgtAssert(_fh->getDimensionality() == _fl->getDimensionality(), "Dimensionality of input volumes must match!");
            cgtAssert(_fh->getSize() == _fl->getSize(), "Size of input volumes must match!");
        }


    void operator() (const tbb::blocked_range<size_t>& range) const {
        for (size_t i = range.begin(); i != range.end(); ++i) {
            float values[2] = { _fl->getElementNormalized(i, 0), _fh->getElementNormalized(i, 0) };
            _histogram->addSample(values);
        }
    }
    protected:
        const ImageRepresentationLocal* _fl;
        const ImageRepresentationLocal* _fh;
        ConcurrentGenericHistogramND<float, 2>* _histogram;
    };

// ================================================================================================

    const std::string LHHistogram::loggerCat_ = "CAMPVis.modules.classification.LHHistogram";

    LHHistogram::LHHistogram()
        : AbstractProcessor()
        , p_intensitiesId("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_gradientsId("InputGradients", "Input Gradient Volume ID", "gradients", DataNameProperty::READ)
        , p_outputFL("OutputFL", "FL Output Volume", "fl", DataNameProperty::WRITE)
        , p_outputFH("OutputFH", "FH Output Volume", "fh", DataNameProperty::WRITE)
    {
        addProperty(p_intensitiesId);
        addProperty(p_gradientsId);
        addProperty(p_outputFL);
        addProperty(p_outputFH);
    }

    LHHistogram::~LHHistogram() {

    }

    void LHHistogram::updateResult(DataContainer& data) {
        ImageRepresentationLocal::ScopedRepresentation intensities(data, p_intensitiesId.getValue());
        GenericImageRepresentationLocal<float, 4>::ScopedRepresentation gradients(data, p_gradientsId.getValue());

        if (intensities != 0 && gradients != 0) {
            ImageData* imgFl = new ImageData(intensities->getDimensionality(), intensities->getSize(), 1);
            ImageRepresentationLocal* fl = GenericImageRepresentationLocal<float, 1>::create(imgFl, 0);

            ImageData* imgFh = new ImageData(intensities->getDimensionality(), intensities->getSize(), 1);
            ImageRepresentationLocal* fh = GenericImageRepresentationLocal<float, 1>::create(imgFh, 0);

            const GenericImageRepresentationLocal<float, 4>* ggg = gradients;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, intensities->getNumElements()), LHGenerator(intensities, ggg, fl, fh, .003f));

            Interval<float> interval = intensities->getNormalizedIntensityRange();
            float mins[2] = { interval.getLeft(), interval.getLeft() };
            float maxs[2] = { interval.getRight(), interval.getRight() };
            size_t numBuckets[2] = { 256, 256 };
            ConcurrentGenericHistogramND<float, 2> lhHistogram(mins, maxs, numBuckets);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, intensities->getNumElements()), LHHistogramGenerator(fl, fh, &lhHistogram));

            // TODO: ugly hack...
            float* tmp = new float[256*256];
            for (size_t i = 0; i < 256*256; ++i)
                tmp[i] = static_cast<float>(lhHistogram.getBuckets()[i]) / static_cast<float>(lhHistogram.getMaxFilling());

            WeaklyTypedPointer wtp(WeaklyTypedPointer::FLOAT, 1, tmp);
            ImageData* imgTex = new ImageData(2, cgt::svec3(256, 256, 1), 1);
            ImageRepresentationGL::create(imgTex, wtp);
            delete [] tmp;

            data.addData("foo", imgTex);
            data.addData(p_outputFH.getValue(), imgFh);
            data.addData(p_outputFL.getValue(), imgFl);
        }
        else {
            LDEBUG("No suitable intensities image found.");
        }
    }

}
