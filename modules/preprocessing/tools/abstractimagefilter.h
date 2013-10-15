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

#ifndef ABSTRACTIMAGEFILTER_H__
#define ABSTRACTIMAGEFILTER_H__

#include <tbb/tbb.h>
#include "tgt/assert.h"

#include <vector>

namespace campvis {
    class ImageRepresentationLocal;

    struct AbstractImageFilter {
        AbstractImageFilter(const ImageRepresentationLocal* input, ImageRepresentationLocal* output)
            : _input(input)
            , _output(output)
        {
            tgtAssert(input != 0, "Input image must not be 0.");
            tgtAssert(output != 0, "Output image must not be 0.");
        }

        virtual void operator() (const tbb::blocked_range<size_t>& range) const = 0;

    protected:
        const ImageRepresentationLocal* _input;
        ImageRepresentationLocal* _output;
    };

// ================================================================================================

    struct ImageFilterMedian : public AbstractImageFilter {
    public:
        ImageFilterMedian(const ImageRepresentationLocal* input, ImageRepresentationLocal* output, size_t kernelSize);

        void operator() (const tbb::blocked_range<size_t>& range) const;

    protected:
        size_t _kernelSize;
    };

// ================================================================================================

    struct ImageFilterGauss : public AbstractImageFilter {
    public:
        ImageFilterGauss(const ImageRepresentationLocal* input, ImageRepresentationLocal* output, size_t kernelSize, float sigma);

        void operator() (const tbb::blocked_range<size_t>& range) const;

    protected:
        size_t _kernelSize;
        size_t _halfKernelSize;
        float _sigma;
        std::vector<float> _kernel;
        float _norm;
    };
}

#endif // ABSTRACTIMAGEFILTER_H__
