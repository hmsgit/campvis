// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
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

#include "clraycaster.h"

#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "kisscl/clruntime.h"
#include "kisscl/commandqueue.h"
#include "kisscl/kernel.h"
#include "kisscl/program.h"
#include "kisscl/memory.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagedatalocal.h"
#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedataconverter.h"

#include "core/classification/simpletransferfunction.h"

namespace campvis {
    const std::string CLRaycaster::loggerCat_ = "CAMPVis.modules.vis.CLRaycaster";

    CLRaycaster::CLRaycaster(GenericProperty<tgt::ivec2>& renderTargetSize)
        : VisualizationProcessor(renderTargetSize)
        , _sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , _entryImageID("entryImageID", "Output Entry Points Image", "", DataNameProperty::READ)
        , _exitImageID("exitImageID", "Output Exit Points Image", "", DataNameProperty::READ)
        , _targetImageID("targetImageID", "Target image ID", "", DataNameProperty::WRITE)
        , _camera("camera", "Camera")
        , _transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _samplingStepSize("samplingStepSize", "Sampling Step Size", .1f, 0.001f, 1.f)
        , _jitterEntryPoints("jitterEntryPoints", "Jitter Entry Points", true)
        , _clContext(0)
        , _clProgram(0)
        , _imgVolume(0)
        , _volumeTimestamp(0)
        , _imgTf(0)
        , _tfTimestamp(0)
        , _texEntryPointsColor(0)
        , _texExitPointsColor(0)
        , _texOutColor(0)
    {
        addProperty(&_sourceImageID);
        addProperty(&_entryImageID);
        addProperty(&_exitImageID);
        addProperty(&_targetImageID);
        addProperty(&_camera);  
        addProperty(&_transferFunction);
        addProperty(&_samplingStepSize);
        addProperty(&_jitterEntryPoints);
    }

    CLRaycaster::~CLRaycaster() {
        delete _imgVolume;
        delete _imgTf;
        delete _texEntryPointsColor;
        delete _texExitPointsColor;
        delete _texOutColor;
    }

    void CLRaycaster::init() {
        VisualizationProcessor::init();

        _clContext = CLRtm.createGlSharingContext();
        if (_clContext != 0) {
            _clProgram = CLRtm.loadProgram(_clContext, "modules/vis/clraycaster.cl");
            _clProgram->setBuildOptions(" -cl-fast-relaxed-math -cl-mad-enable");
            _clProgram->build();
        }
    }

    void CLRaycaster::deinit() {
        CLRtm.dispose(_clProgram);
        delete _clContext;
        VisualizationProcessor::deinit();
    }

    void CLRaycaster::process(DataContainer& data) {
        if (_clContext == 0 || _clProgram == 0) 
            return;

        DataContainer::ScopedTypedData<ImageDataLocal> img(data, _sourceImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> entryPoints(data, _entryImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> exitPoints(data, _exitImageID.getValue());

        if (img != 0 && entryPoints != 0 && exitPoints != 0) {
            if (img->getDimensionality() == 3) {
                if (entryPoints->getSize() == exitPoints->getSize()) {
                    tgt::svec3 dims(entryPoints->getColorTexture()->getDimensions());

                    if (_invalidationLevel.isInvalidShader()) {
                        _clProgram->build();
                    }

                    if (img.getDataHandle().getTimestamp() != _volumeTimestamp) {
                        WeaklyTypedPointer wtp = img->getWeaklyTypedPointer();
                        _imgVolume = new kisscl::Image(_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, img->getSize(), wtp.getClChannelOrder(), wtp.getClChannelType(), wtp._pointer);
                        _volumeTimestamp = img.getDataHandle().getTimestamp();
                    }

                    // upload TF
                    const tgt::Texture* tf = _transferFunction.getTF()->getTexture();
                    delete _imgTf;
                    _imgTf = new kisscl::Image(_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, tf);

                    // bind shared textures
                    delete _texEntryPointsColor;
                    _texEntryPointsColor = new kisscl::GLTexture(_clContext, CL_MEM_READ_ONLY, entryPoints->getColorTexture());
                    delete _texExitPointsColor;
                    _texExitPointsColor = new kisscl::GLTexture(_clContext, CL_MEM_READ_ONLY, exitPoints->getColorTexture());
                    delete _texOutColor;
                    ImageDataRenderTarget* rt = new ImageDataRenderTarget(dims);
                    _texOutColor = new kisscl::GLTexture(_clContext, CL_MEM_WRITE_ONLY, rt->getColorTexture());


                    // prepare kernel and stuff command queue
                    kisscl::CommandQueue* cq = CLRtm.getCommandQueue(_clContext);
                    kisscl::Kernel* kernel = _clProgram->getKernel("clraycaster");

                    if (kernel != 0) {
                        kernel->setArguments(
                            *_imgVolume, 
                            *_imgTf, 
                            *_texEntryPointsColor, 
                            *_texExitPointsColor, 
                            *_texOutColor, 
                            _samplingStepSize.getValue(), 
                            _transferFunction.getTF()->getIntensityDomain().x, 
                            _transferFunction.getTF()->getIntensityDomain().y);

                        cq->enqueueAcquireGLObject(kisscl::GLTextureList(*_texEntryPointsColor, *_texExitPointsColor, *_texOutColor));
                        cq->enqueueKernel(kernel, dims.xy());
                        cq->enqueueReleaseGLObject(kisscl::GLTextureList(*_texEntryPointsColor, *_texExitPointsColor, *_texOutColor));

                        cq->finish();
                    }
                    else {
                        LERROR("Kernel 'clraycaster' not found");
                        return;
                    }

                    LGL_ERROR;
                    data.addData(_targetImageID.getValue(), rt);
                    _targetImageID.issueWrite();
                }
                else {
                    LERROR("Entry-/Exitpoints texture size does not match.");
                }
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

    std::string CLRaycaster::generateHeader() const {
        std::string toReturn;
        return toReturn;
    }

}
