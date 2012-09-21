// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

namespace TUMVis {
    class LHGenerator {
    public:
        LHGenerator(const ImageDataLocal* intensities, const GenericImageDataLocal<float, 4>* gradients, ImageDataLocal* fh, ImageDataLocal* fl, float epsilon)
            : _intensities(intensities)
            , _gradients(gradients)
            , _fh(fh)
            , _fl(fl)
            , _epsilon(epsilon)
        {
            tgtAssert(_intensities->getDimensionality() == _gradients->getDimensionality(), "Dimensionality of intensities volumes must match!");
            tgtAssert(_intensities->getSize() == _gradients->getSize(), "Size of intensities volumes must match!");
        }

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
            tgt::vec3 stepSize(1.f);
            tgt::vec3 size(_intensities->getSize());

            while (gradient1.w > _epsilon) {
                tgt::vec4 gradient2 = getGradientLinear(position + tgt::normalize(gradient1.xyz()) * stepSize);
                position += tgt::normalize((gradient1 + gradient2).xyz()) * stepSize;
                gradient1 = getGradientLinear(position);

                if (tgt::hor(tgt::lessThan(position, tgt::vec3::zero)) || tgt::hor(tgt::greaterThan(position, size)))
                    break;
            }

            return _intensities->getElementNormalizedLinear(position, 0);;
        }

        void operator() (const tbb::blocked_range<size_t>& range) const {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                tgt::svec3 pos = _intensities->indexToPosition(i);
                const tgt::svec3& size = _intensities->getSize();

                const tgt::vec4& gradient = _gradients->getElement(i);
                float fl = _intensities->getElementNormalized(pos, 0);
                float fh = fl;

                float forwardIntensity = integrateHeun(tgt::vec3(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z)), gradient);
                float backwardIntensity = integrateHeun(tgt::vec3(static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z)), gradient * -1.f);

                fh = std::max(forwardIntensity, backwardIntensity);
                fl = std::min(forwardIntensity, backwardIntensity);

                _fl->setElementNormalized(pos, 0, fl);
                _fh->setElementNormalized(pos, 0, fh);
            }
        }

    protected:
        const ImageDataLocal* _intensities;
        const GenericImageDataLocal<float, 4>* _gradients;
        ImageDataLocal* _fh;
        ImageDataLocal* _fl;
        float _epsilon;
    };

// ================================================================================================

    const std::string LHHistogram::loggerCat_ = "TUMVis.modules.classification.LHHistogram";

    LHHistogram::LHHistogram()
        : AbstractProcessor()
        , _inputVolume("InputVolume", "Input Volume ID", "volume", DataNameProperty::READ)
        , _inputGradients("InputGradients", "Input Gradient Volume ID", "gradients", DataNameProperty::READ)
        , _outputFH("OutputFH", "FH Output Volume", "fh", DataNameProperty::WRITE)
        , _outputFL("OutputFL", "FL Output Volume", "fl", DataNameProperty::WRITE)
    {
        addProperty(&_inputVolume);
        addProperty(&_inputGradients);
        addProperty(&_outputFH);
        addProperty(&_outputFL);
    }

    LHHistogram::~LHHistogram() {

    }

    void LHHistogram::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataLocal> intensities(data, _inputVolume.getValue());
        DataContainer::ScopedTypedData< GenericImageDataLocal<float, 4> > gradients(data, _inputGradients.getValue());

        if (intensities != 0 && gradients != 0) {
            ImageDataLocal* fh = intensities->clone();
            ImageDataLocal* fl = intensities->clone();
            tbb::parallel_for(tbb::blocked_range<size_t>(0, intensities->getNumElements()), LHGenerator(intensities, gradients, fh, fl, .01f));

            data.addData(_outputFH.getValue(), fh);
            data.addData(_outputFL.getValue(), fl);
            _outputFH.issueWrite();
            _outputFL.issueWrite();
        }
        else {
            LDEBUG("No suitable intensities image found.");
        }

        _invalidationLevel.setValid();
    }

}
