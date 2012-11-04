// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "lhhistogram.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "tbb/include/tbb/tbb.h"

#include "core/datastructures/genericimagedatalocal.h"
#include "core/datastructures/imagedatagl.h"
#include "core/tools/concurrenthistogram.h"

namespace campvis {
    class LHGenerator {
    public:
        LHGenerator(const ImageDataLocal* intensities, const GenericImageDataLocal<float, 4>* gradients, ImageDataLocal* fl, ImageDataLocal* fh, float epsilon)
            : _intensities(intensities)
            , _gradients(gradients)
            , _fl(fl)
            , _fh(fh)
            , _epsilon(epsilon)
        {
            tgtAssert(_intensities->getDimensionality() == _gradients->getDimensionality(), "Dimensionality of intensities volumes must match!");
            tgtAssert(_intensities->getSize() == _gradients->getSize(), "Size of intensities volumes must match!");
        }

        void operator() (const tbb::blocked_range<size_t>& range) const {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                tgt::svec3 pos = _intensities->indexToPosition(i);
                const tgt::svec3& size = _intensities->getSize();

                const tgt::vec4& gradient = _gradients->getElement(i);
                float fl = _intensities->getElementNormalized(i, 0);
                float fh = fl;

                float forwardIntensity = integrateHeun(tgt::vec3(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z)), gradient);
                float backwardIntensity = integrateHeun(tgt::vec3(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z)), gradient * -1.f);

                fh = std::max(forwardIntensity, backwardIntensity);
                fl = std::min(forwardIntensity, backwardIntensity);

                _fl->setElementNormalized(i, 0, fl);
                _fh->setElementNormalized(i, 0, fh);
            }
        }

    protected:
        tgt::vec4 getGradientLinear(const tgt::vec3& position) const {
            tgt::vec4 result;
            result.x = _gradients->getElementNormalizedLinear(position, 0);
            result.y = _gradients->getElementNormalizedLinear(position, 1);
            result.z = _gradients->getElementNormalizedLinear(position, 2);
            result.w = _gradients->getElementNormalizedLinear(position, 3);
            return result;
        }

        float integrateHeun(tgt::vec3 position, const tgt::vec4& direction) const {
            tgt::vec4 gradient1 = direction;
            tgt::vec3 stepSize(.25f);
            tgt::vec3 size(_intensities->getSize());
            size_t numSteps = 0;

            while (gradient1.w > _epsilon) {
                tgt::vec4 gradient2 = getGradientLinear(position + tgt::normalize(gradient1.xyz()) * stepSize/2.f);
                position += tgt::normalize((gradient1 + gradient2).xyz()) * stepSize;
                gradient1 = getGradientLinear(position);
                ++numSteps;

                if (numSteps > 128 || tgt::hor(tgt::lessThan(position, tgt::vec3::zero)) || tgt::hor(tgt::greaterThan(position, size)))
                    break;
            }

            return _intensities->getElementNormalizedLinear(position, 0);;
        }

        const ImageDataLocal* _intensities;
        const GenericImageDataLocal<float, 4>* _gradients;
        ImageDataLocal* _fl;
        ImageDataLocal* _fh;
        float _epsilon;
    };

// ================================================================================================

    class LHHistogramGenerator {
    public:
        LHHistogramGenerator(const ImageDataLocal* fl, const ImageDataLocal* fh, ConcurrentGenericHistogramND<float, 2>* histogram)
            : _fl(fl)
            , _fh(fh)
            , _histogram(histogram)
        {
            tgtAssert(_fh->getDimensionality() == _fl->getDimensionality(), "Dimensionality of input volumes must match!");
            tgtAssert(_fh->getSize() == _fl->getSize(), "Size of input volumes must match!");
        }


    void operator() (const tbb::blocked_range<size_t>& range) const {
        for (size_t i = range.begin(); i != range.end(); ++i) {
            float values[2] = { _fl->getElementNormalized(i, 0), _fh->getElementNormalized(i, 0) };
            _histogram->addSample(values);
        }
    }
    protected:
        const ImageDataLocal* _fl;
        const ImageDataLocal* _fh;
        ConcurrentGenericHistogramND<float, 2>* _histogram;
    };

// ================================================================================================

    const std::string LHHistogram::loggerCat_ = "CAMPVis.modules.classification.LHHistogram";

    LHHistogram::LHHistogram()
        : AbstractProcessor()
        , p_inputVolume("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , p_inputGradients("InputGradients", "Input Gradient Volume ID", "gradients", DataNameProperty::READ)
        , p_outputFL("OutputFL", "FL Output Volume", "fl", DataNameProperty::WRITE)
        , p_outputFH("OutputFH", "FH Output Volume", "fh", DataNameProperty::WRITE)
    {
        addProperty(&p_inputVolume);
        addProperty(&p_inputGradients);
        addProperty(&p_outputFL);
        addProperty(&p_outputFH);
    }

    LHHistogram::~LHHistogram() {

    }

    void LHHistogram::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataLocal> intensities(data, p_inputVolume.getValue());
        DataContainer::ScopedTypedData< GenericImageDataLocal<float, 4> > gradients(data, p_inputGradients.getValue());

        if (intensities != 0 && gradients != 0) {
            ImageDataLocal* fl = intensities->clone();
            ImageDataLocal* fh = intensities->clone();
            tbb::parallel_for(tbb::blocked_range<size_t>(0, intensities->getNumElements()), LHGenerator(intensities, gradients, fl, fh, .006f));

            float mins[2] = { 0.f, 0.f };
            float maxs[2] = { .1f, .1f };
            size_t numBuckets[2] = { 256, 256 };
            ConcurrentGenericHistogramND<float, 2> lhHistogram(mins, maxs, numBuckets);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, intensities->getNumElements()), LHHistogramGenerator(fl, fh, &lhHistogram));

            // TODO: ugly hack...
            float* tmp = new float[256*256];
            for (size_t i = 0; i < 256*256; ++i)
                tmp[i] = static_cast<float>(lhHistogram.getBuckets()[i]) / static_cast<float>(lhHistogram.getMaxFilling());

            WeaklyTypedPointer wtp(WeaklyTypedPointer::FLOAT, 1, tmp);
            ImageDataGL* tex = new ImageDataGL(2, tgt::svec3(256, 256, 1), wtp);
            delete [] tmp;

            data.addData("foo", tex);
            data.addData(p_outputFH.getValue(), fh);
            data.addData(p_outputFL.getValue(), fl);
            p_outputFH.issueWrite();
            p_outputFL.issueWrite();
        }
        else {
            LDEBUG("No suitable intensities image found.");
        }

        _invalidationLevel.setValid();
    }

}
