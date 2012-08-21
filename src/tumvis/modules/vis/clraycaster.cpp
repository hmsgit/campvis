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

#include "clraycaster.h"

#include "tgt/logmanager.h"
#include "tgt/quadrenderer.h"
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

namespace TUMVis {
    const std::string CLRaycaster::loggerCat_ = "TUMVis.modules.vis.CLRaycaster";

    CLRaycaster::CLRaycaster(GenericProperty<tgt::ivec2>& renderTargetSize)
        : VisualizationProcessor(renderTargetSize)
        , _sourceImageID("sourceImageID", "Input Image", "")
        , _entryImageID("entryImageID", "Output Entry Points Image", "")
        , _exitImageID("exitImageID", "Output Exit Points Image", "")
        , _targetImageID("targetImageID", "Target image ID", "")
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
        _clProgram = CLRtm.loadProgram(_clContext, "modules/vis/clraycaster.cl");
        _clProgram->setBuildOptions(" -cl-fast-relaxed-math -cl-mad-enable");
        _clProgram->build();
    }

    void CLRaycaster::deinit() {
        CLRtm.dispose(_clProgram);
        VisualizationProcessor::deinit();
    }

    void CLRaycaster::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataLocal> img(data, _sourceImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> entryPoints(data, _entryImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> exitPoints(data, _exitImageID.getValue());

        if (img != 0 && entryPoints != 0 && exitPoints != 0) {
            if (img->getDimensionality() == 3) {
                if (_invalidationLevel.isInvalidShader()) {
                    _clProgram->build();
                }

                if (img.getDataHandle()->getTimestamp() != _volumeTimestamp) {
                    WeaklyTypedPointer wtp = img->getWeaklyTypedPointer();
                    _imgVolume = new kisscl::Image(_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, img->getSize(), wtp.getClChannelOrder(), wtp.getClChannelType(), wtp._pointer);
                    _volumeTimestamp = img.getDataHandle()->getTimestamp();
                }
                
                // upload TF
                const tgt::Texture* tf = _transferFunction.getTF()->getTexture();
                delete _imgTf;
                _imgTf = new kisscl::Image(_clContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, tf);

                // bind shared textures
                delete _texEntryPointsColor;
                _texEntryPointsColor = new kisscl::SharedTexture(_clContext, CL_MEM_READ_ONLY, entryPoints->getColorTexture());
                delete _texExitPointsColor;
                _texExitPointsColor = new kisscl::SharedTexture(_clContext, CL_MEM_READ_ONLY, exitPoints->getColorTexture());
                delete _texOutColor;
                ImageDataRenderTarget* rt = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
                _texOutColor = new kisscl::SharedTexture(_clContext, CL_MEM_WRITE_ONLY, rt->getColorTexture());


                // prepare kernel and stuff command queue
                kisscl::CommandQueue* cq = CLRtm.getCommandQueue(_clContext);
                kisscl::Kernel* kernel = _clProgram->getKernel("clraycaster");

                if (kernel != 0) {
                    rt->activate();
                    rt->deactivate();

                    LGL_ERROR;
                    glFinish();

                    kernel->setMemoryArgument(0, _imgVolume);
                    kernel->setMemoryArgument(1, _imgTf);
                    kernel->setMemoryArgument(2, _texEntryPointsColor);
                    kernel->setMemoryArgument(3, _texExitPointsColor);
                    kernel->setMemoryArgument(4, _texOutColor);
                    kernel->setArgument(5, _samplingStepSize.getValue());
                    kernel->setArgument(6, _transferFunction.getTF()->getIntensityDomain().x);
                    kernel->setArgument(7, _transferFunction.getTF()->getIntensityDomain().y);

                    cq->enqueueAcquireGLObject(_texEntryPointsColor);
                    cq->enqueueAcquireGLObject(_texExitPointsColor);
                    cq->enqueueAcquireGLObject(_texOutColor);

                    cq->enqueueKernel(kernel, entryPoints->getSize().xy());

                    cq->enqueueReleaseGLObject(_texEntryPointsColor);
                    cq->enqueueReleaseGLObject(_texExitPointsColor);
                    cq->enqueueReleaseGLObject(_texOutColor);

                    cq->finish();
                }
                else {
                    LERROR("Kernel 'clraycaster' not found");
                    return;
                }
                /*
                const int size = 1048576;
                const int mem_size = sizeof(float)*size;
                float* src_a_h = new float[size];
                float* src_b_h = new float[size];
                float* res_h = new float[size];
                // Initialize both vectors
                for (int i = 0; i < size; i++) {
                    src_a_h[i] = src_b_h[i] = (float) i;
                }

                kisscl::Buffer a(_clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_a_h);
                kisscl::Buffer b(_clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_b_h);
                kisscl::Buffer res(_clContext, CL_MEM_WRITE_ONLY, mem_size, res_h);

                kisscl::Kernel* kernel = _clProgram->getKernel("vector_add_gpu");
                if (kernel != 0) {
                    kernel->setMemoryArgument(0, &a);
                    kernel->setMemoryArgument(1, &b);
                    kernel->setMemoryArgument(2, &res);
                    kernel->setArgument(3, size);

                    cq->enqueueKernel(kernel, size, 512);
                    cq->enqueueRead(&res, res_h);
                    cq->finish();
                }*/

                LGL_ERROR;
                data.addData(_targetImageID.getValue(), rt);
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
