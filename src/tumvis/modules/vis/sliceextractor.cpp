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

#include "sliceextractor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedataconverter.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace TUMVis {
    const std::string SliceExtractor::loggerCat_ = "TUMVis.modules.vis.SliceExtractor";

    SliceExtractor::SliceExtractor(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , _sourceImageID("sourceImageID", "Input Image", "")
        , _targetImageID("targetImageID", "Output Image", "")
        , _sliceNumber("sliceNumber", "Slice Number", 0, 0, 0)
        , _transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _shader(0)
    {
        addProperty(&_sourceImageID);
        addProperty(&_targetImageID);
        addProperty(&_sliceNumber);
        addProperty(&_transferFunction);
    }

    SliceExtractor::~SliceExtractor() {

    }

    void SliceExtractor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/sliceextractor.frag", "", false);
    }

    void SliceExtractor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void SliceExtractor::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataLocal> img(data, _sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                updateProperties(img);
                const tgt::svec3& imgSize = img->getSize();
                ImageDataLocal* slice = img->getSubImage(tgt::svec3(0, 0, _sliceNumber.getValue()), tgt::svec3(imgSize.x-1, imgSize.y-1, _sliceNumber.getValue()));
                ImageDataGL* glData = ImageDataConverter::tryConvert<ImageDataGL>(slice);
                ImageDataRenderTarget* rt = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));

                _shader->activate();
                tgt::TextureUnit inputUnit, tfUnit;
                glData->bind(_shader, inputUnit);
                _transferFunction.getTF()->bind(_shader, tfUnit);

                rt->activate();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                _shader->setAttributeLocation(0, "in_Position");
                _shader->setAttributeLocation(1, "in_TexCoord");
                QuadRdr.renderQuad();
                rt->deactivate();

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();

                data.addData(_targetImageID.getValue(), rt);
                delete slice;
                delete glData;
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        _invalidationLevel.setValid();
    }

    void SliceExtractor::updateProperties(const ImageData* img) {
        const tgt::svec3& imgSize = img->getSize();
        if (_sliceNumber.getMaxValue() != imgSize.z - 1){
            _sliceNumber.setMaxValue(imgSize.z - 1);
        }
    }

}
