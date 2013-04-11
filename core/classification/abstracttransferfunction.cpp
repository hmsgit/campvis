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

#include "abstracttransferfunction.h"

#include "tbb/tbb.h"
#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagerepresentationlocal.h"

namespace campvis {
    
    class IntensityHistogramGenerator {
    public:
        IntensityHistogramGenerator(const ImageRepresentationLocal* intensityData, AbstractTransferFunction::IntensityHistogramType* histogram)
            : _intensityData(intensityData)
            , _histogram(histogram)
        {}

        void operator() (const tbb::blocked_range<size_t>& range) const {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                float value = _intensityData->getElementNormalized(i, 0);
                _histogram->addSample(&value);
            }
        }

    protected:
        const ImageRepresentationLocal* _intensityData;
        AbstractTransferFunction::IntensityHistogramType* _histogram;
    };

// ================================================================================================

    const std::string AbstractTransferFunction::loggerCat_ = "CAMPVis.core.classification.AbstractTransferFunction";

    AbstractTransferFunction::AbstractTransferFunction(const tgt::svec3& size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/)
        : _size(size)
        , _intensityDomain(intensityDomain)
        , _texture(0)
        , _imageHandle(0)
        , _intensityHistogram(0)
    {
        _dirtyTexture = false;
        _dirtyHistogram = false;
    }

    AbstractTransferFunction::~AbstractTransferFunction() {
        if (_texture != 0)
            LWARNING("Called AbstractTransferFunction dtor without proper deinitialization - you just wasted resources!");
        delete _intensityHistogram;
    }

    void AbstractTransferFunction::deinit() {
        delete _texture;
        _texture = 0;
    }

    void AbstractTransferFunction::bind(tgt::Shader* shader, const tgt::TextureUnit& texUnit, const std::string& transFuncUniform /*= "_transferFunction"*/, const std::string& transFuncParamsUniform /*= "_transferFunctionParameters"*/) {
        tgtAssert(shader != 0, "Shader must not be 0.");

        {
            // TODO:    lock here or in createTexture?
            tbb::mutex::scoped_lock lock(_localMutex);
            if (_texture == 0 || _dirtyTexture) {
                shader->deactivate();
                createTexture();
                shader->activate();
            }
        }

        texUnit.activate();
        _texture->bind();

        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);
        shader->setUniform(transFuncUniform, texUnit.getUnitNumber());
        switch (getDimensionality()) {
            case 1:
                shader->setUniform(transFuncParamsUniform + "._intensityDomain", tgt::vec2(_intensityDomain));
                break;
            case 2:
                shader->setUniform(transFuncParamsUniform + "._intensityDomainX", tgt::vec2(_intensityDomain));
                break;
            default:
                tgtAssert(false, "Unsupported TF dimensionality!");
                break;
        }
        
        shader->setIgnoreUniformLocationError(tmp);
    }

    void AbstractTransferFunction::uploadTexture() {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            if (_texture == 0 || _dirtyTexture) {
                createTexture();
            }
        }
    }

    void AbstractTransferFunction::setIntensityDomain(const tgt::vec2& newDomain) {
        tgtAssert(newDomain.x <= newDomain.y, "Intensity domain is not a valid interval.");
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _intensityDomain = newDomain;
        }
        _dirtyHistogram = true;
        s_changed();
    }

    const tgt::vec2& AbstractTransferFunction::getIntensityDomain() const {
        return _intensityDomain;
    }

    const tgt::Texture* AbstractTransferFunction::getTexture() {
        // TODO:    lock here or in createTexture?
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            if (_texture == 0 || _dirtyTexture) {
                createTexture();
            }
        }
        return _texture;
    }

    DataHandle AbstractTransferFunction::getImageHandle() const {
        return _imageHandle;
    }

    void AbstractTransferFunction::setImageHandle(DataHandle imageHandle) {
        tgtAssert(
            imageHandle.getData() == 0 || dynamic_cast<const ImageData*>(imageHandle.getData()) != 0, 
            "The data in the image handle must either be 0 or point to a valid ImageData object!");

        _imageHandle = imageHandle;
        _dirtyHistogram = true;
        s_imageHandleChanged();
    }

    void AbstractTransferFunction::computeIntensityHistogram() const {
        delete _intensityHistogram;
        _intensityHistogram = 0;

        ImageRepresentationLocal::ScopedRepresentation repLocal(_imageHandle);
        if (repLocal != 0) {
            float mins = _intensityDomain.x;
            float maxs = _intensityDomain.y;
            size_t numBuckets = std::min(WeaklyTypedPointer::numBytes(repLocal->getWeaklyTypedPointer()._baseType) << 8, static_cast<size_t>(512));
            _intensityHistogram = new IntensityHistogramType(&mins, &maxs, &numBuckets);
            tbb::parallel_for(tbb::blocked_range<size_t>(0, repLocal->getNumElements()), IntensityHistogramGenerator(repLocal, _intensityHistogram));
        }

        _dirtyHistogram = false;
    }

    const AbstractTransferFunction::IntensityHistogramType* AbstractTransferFunction::getIntensityHistogram() const {
        if (_dirtyHistogram) {
            computeIntensityHistogram();
        }

        return _intensityHistogram;
    }


}