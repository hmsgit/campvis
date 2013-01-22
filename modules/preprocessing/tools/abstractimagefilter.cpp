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

#include "abstractimagefilter.h"


#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include <algorithm>

namespace campvis {

    ImageFilterMedian::ImageFilterMedian(const ImageRepresentationLocal* input, ImageRepresentationLocal* output, size_t kernelSize)
        : AbstractImageFilter(input, output)
        , _kernelSize(kernelSize)
    {
        tgtAssert(kernelSize > 0, "Kernel Size must be greater 0.");
    }

    void ImageFilterMedian::operator()(const tbb::blocked_range<size_t>& range) const {
        size_t halfKernelDim = static_cast<size_t>(_kernelSize / 2);
        const tgt::svec3& size = _input->getSize();
        
        for (size_t index = range.begin(); index < range.end(); ++index) {
            tgt::svec3 position = _input->getParent()->indexToPosition(index);

            size_t zmin = position.z >= halfKernelDim ? position.z - halfKernelDim : 0;
            size_t zmax = std::min(position.z+halfKernelDim, size.z-1);
            size_t ymin = position.y >= halfKernelDim ? position.y - halfKernelDim : 0;
            size_t ymax = std::min(position.y+halfKernelDim, size.y-1);
            size_t xmin = position.x >= halfKernelDim ? position.x - halfKernelDim : 0;
            size_t xmax = std::min(position.x+halfKernelDim, size.x-1);

            tgt::svec3 npos;
            std::vector<float> values;
            for (npos.z=zmin; npos.z<=zmax; npos.z++) {
                for (npos.y=ymin; npos.y<=ymax; npos.y++) {
                    for (npos.x=xmin; npos.x<=xmax; npos.x++) {
                        values.push_back(_input->getElementNormalized(npos, 0));
                    }
                }
            }
            size_t medianPosition = values.size() / 2;
            std::nth_element(values.begin(), values.begin() + medianPosition, values.end());
            _output->setElementNormalized(index, 0, values[medianPosition]);
        }
    }

// ================================================================================================

    ImageFilterGauss::ImageFilterGauss(const ImageRepresentationLocal* input, ImageRepresentationLocal* output, size_t kernelSize, float sigma)
        : AbstractImageFilter(input, output)
        , _kernelSize(kernelSize)
        , _halfKernelSize(kernelSize / 2)
        , _sigma(sigma)
        , _norm(0.f)
    {
        tgtAssert(kernelSize > 0, "Kernel Size must be greater 0.");

        // compute Gauss kernel and corresponding norm
        // it is sufficient to compute only one half of the 1D kernel
        _kernel.resize(_halfKernelSize + 1, 0.f);
        for (size_t i = 0; i <= _halfKernelSize; ++i) {
            float f = static_cast<float>(i);
            _kernel[i] = exp(-(f * f) / (2.f * _sigma * _sigma));
            _norm += _kernel[i];
        }

        _norm = (2.f * _norm) - _kernel[0];
    }


    void ImageFilterGauss::operator()(const tbb::blocked_range<size_t>& range) const {
        size_t halfKernelDim = static_cast<size_t>(_kernelSize / 2);
        const tgt::svec3& size = _input->getSize();

        for (size_t index = range.begin(); index < range.end(); ++index) {
            tgt::svec3 position = _input->getParent()->indexToPosition(index);
            tgt::svec3 npos = position;
            float sum = 0.f;

            size_t zmin = position.z >= halfKernelDim ? position.z - halfKernelDim : 0;
            size_t zmax = std::min(position.z+halfKernelDim, size.z-1);
            for (npos.z = zmin; npos.z <= zmax; npos.z++) {
                int i = abs(static_cast<int>(position.z) - static_cast<int>(npos.z));
                sum += _input->getElementNormalized(npos, 0) * _kernel[i];
            }
            npos.z = position.z;

            size_t ymin = position.y >= halfKernelDim ? position.y - halfKernelDim : 0;
            size_t ymax = std::min(position.y+halfKernelDim, size.y-1);
            for (npos.y=ymin; npos.y<=ymax; npos.y++) {
                int i = abs(static_cast<int>(position.y) - static_cast<int>(npos.y));
                sum += _input->getElementNormalized(npos, 0) * _kernel[i];
            }
            npos.y = position.y;

            size_t xmin = position.x >= halfKernelDim ? position.x - halfKernelDim : 0;
            size_t xmax = std::min(position.x+halfKernelDim, size.x-1);
            for (npos.x=xmin; npos.x<=xmax; npos.x++) {
                int i = abs(static_cast<int>(position.x) - static_cast<int>(npos.x));
                sum += _input->getElementNormalized(npos, 0) * _kernel[i];
            }

            sum /= 3.f * _norm;
            _output->setElementNormalized(index, 0, sum);
        }
    }

}
