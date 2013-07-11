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

#include "imageseriessplitter.h"

#include "core/datastructures/imageseries.h"

namespace campvis {
    const std::string ImageSeriesSplitter::loggerCat_ = "CAMPVis.modules.io.ImageSeriesSplitter";

    ImageSeriesSplitter::ImageSeriesSplitter() 
        : AbstractProcessor()
        , p_inputID("InputID", "Input Image Series ID", "input", DataNameProperty::READ, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES)
        , p_outputID("OutputID", "Output Image ID", "output", DataNameProperty::WRITE)
        , p_imageIndex("ImageIndex", "Image to Select", 0, 0, 0)
    {
        addProperty(&p_inputID);
        addProperty(&p_outputID);
        addProperty(&p_imageIndex);
    }

    ImageSeriesSplitter::~ImageSeriesSplitter() {

    }

    void ImageSeriesSplitter::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageSeries> series(data, p_inputID.getValue());
        if (series != 0) {
            if (hasInvalidProperties()) {
                p_imageIndex.setMaxValue(series->getNumImages());
            }
            if (p_imageIndex.getValue() < static_cast<int>(series->getNumImages())) {
                data.addDataHandle(p_outputID.getValue(), series->getImage(p_imageIndex.getValue()));
                p_outputID.issueWrite();
            }
        }

        validate(INVALID_RESULT);
    }
}