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
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"


#include "core/classification/simpletransferfunction.h"

namespace campvis {
    const std::string CLRaycaster::loggerCat_ = "CAMPVis.modules.vis.CLRaycaster";

    CLRaycaster::CLRaycaster(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , _sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , _entryImageID("entryImageID", "Output Entry Points Image", "", DataNameProperty::READ)
        , _exitImageID("exitImageID", "Output Exit Points Image", "", DataNameProperty::READ)
        , _targetImageID("targetImageID", "Target image ID", "", DataNameProperty::WRITE)
        , _camera("camera", "Camera")
        , _transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _samplingStepSize("samplingStepSize", "Sampling Step Size", .1f, 0.001f, 1.f, 0.001f)
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
            _clProgram = CLRtm.loadProgram(_clContext, "modules/opencl/cl/clraycaster.cl");
            _clProgram->setBuildOptions(" -cl-fast-relaxed-math -cl-mad-enable");// -g -s \"C:\\Users\\Christian\\Documents\\TUM\\code\\campvis\\modules\\opencl\\cl\\clraycaster.cl\"");
            _clProgram->build();
        }

    }

    void CLRaycaster::deinit() {
        CLRtm.dispose(_clProgram);
        delete _clContext;
        VisualizationProcessor::deinit();
    }


    // Helper function to get OpenCL image format string (channel order and type) from constant
    // *********************************************************************
    const char* oclImageFormatString(cl_uint uiImageFormat)
    {
        // cl_channel_order 
        if (uiImageFormat == CL_R)return "CL_R";  
        if (uiImageFormat == CL_A)return "CL_A";  
        if (uiImageFormat == CL_RG)return "CL_RG";  
        if (uiImageFormat == CL_RA)return "CL_RA";  
        if (uiImageFormat == CL_RGB)return "CL_RGB";
        if (uiImageFormat == CL_RGBA)return "CL_RGBA";  
        if (uiImageFormat == CL_BGRA)return "CL_BGRA";  
        if (uiImageFormat == CL_ARGB)return "CL_ARGB";  
        if (uiImageFormat == CL_INTENSITY)return "CL_INTENSITY";  
        if (uiImageFormat == CL_LUMINANCE)return "CL_LUMINANCE";  

        // cl_channel_type 
        if (uiImageFormat == CL_SNORM_INT8)return "CL_SNORM_INT8";
        if (uiImageFormat == CL_SNORM_INT16)return "CL_SNORM_INT16";
        if (uiImageFormat == CL_UNORM_INT8)return "CL_UNORM_INT8";
        if (uiImageFormat == CL_UNORM_INT16)return "CL_UNORM_INT16";
        if (uiImageFormat == CL_UNORM_SHORT_565)return "CL_UNORM_SHORT_565";
        if (uiImageFormat == CL_UNORM_SHORT_555)return "CL_UNORM_SHORT_555";
        if (uiImageFormat == CL_UNORM_INT_101010)return "CL_UNORM_INT_101010";
        if (uiImageFormat == CL_SIGNED_INT8)return "CL_SIGNED_INT8";
        if (uiImageFormat == CL_SIGNED_INT16)return "CL_SIGNED_INT16";
        if (uiImageFormat == CL_SIGNED_INT32)return "CL_SIGNED_INT32";
        if (uiImageFormat == CL_UNSIGNED_INT8)return "CL_UNSIGNED_INT8";
        if (uiImageFormat == CL_UNSIGNED_INT16)return "CL_UNSIGNED_INT16";
        if (uiImageFormat == CL_UNSIGNED_INT32)return "CL_UNSIGNED_INT32";
        if (uiImageFormat == CL_HALF_FLOAT)return "CL_HALF_FLOAT";
        if (uiImageFormat == CL_FLOAT)return "CL_FLOAT";

        // unknown constant
        return "Unknown";
    }

    void CLRaycaster::process(DataContainer& data) {
        if (_clContext == 0 || _clProgram == 0) 
            return;

        ImageRepresentationLocal::ScopedRepresentation img(data, _sourceImageID.getValue());
        ScopedTypedData<RenderData> entryPoints(data, _entryImageID.getValue());
        ScopedTypedData<RenderData> exitPoints(data, _exitImageID.getValue());

        if (img != 0 && entryPoints != 0 && exitPoints != 0) {
            if (img->getDimensionality() == 3) {
                if (entryPoints->getColorTexture()->getSize() == exitPoints->getColorTexture()->getSize()) {
                    tgt::svec3 dims(entryPoints->getColorTexture()->getSize());

                    if (hasInvalidShader()) {
                        _clProgram->build();
                    }

// ================================================================================================
                    // Determine and show image format support 
//                     cl_uint uiNumSupportedFormats = 0;
// 
//                     // 2D
//                     clGetSupportedImageFormats(_clContext->getId(), CL_MEM_READ_ONLY, 
//                         CL_MEM_OBJECT_IMAGE2D,   
//                         NULL, NULL, &uiNumSupportedFormats);
//                     cl_image_format* ImageFormats = new cl_image_format[uiNumSupportedFormats];
//                     clGetSupportedImageFormats(_clContext->getId(), CL_MEM_READ_ONLY, 
//                         CL_MEM_OBJECT_IMAGE2D,   
//                         uiNumSupportedFormats, ImageFormats, NULL);
//                     printf("  ---------------------------------\n");
//                     printf("  2D Image Formats Supported (%u)\n", uiNumSupportedFormats); 
//                     printf("  ---------------------------------\n");
//                     printf("  %-6s%-16s%-22s\n\n", "#", "Channel Order", "Channel Type"); 
//                     for(unsigned int i = 0; i < uiNumSupportedFormats; i++) 
//                     {  
//                         printf("  %-6u%-16s%-22s\n", (i + 1),
//                             oclImageFormatString(ImageFormats[i].image_channel_order), 
//                             oclImageFormatString(ImageFormats[i].image_channel_data_type));
//                     }
//                     printf("\n"); 
//                     delete [] ImageFormats;
// 
//                     // 3D
//                     clGetSupportedImageFormats(_clContext->getId(), CL_MEM_READ_ONLY, 
//                         CL_MEM_OBJECT_IMAGE3D,   
//                         NULL, NULL, &uiNumSupportedFormats);
//                     ImageFormats = new cl_image_format[uiNumSupportedFormats];
//                     clGetSupportedImageFormats(_clContext->getId(), CL_MEM_READ_ONLY, 
//                         CL_MEM_OBJECT_IMAGE3D,   
//                         uiNumSupportedFormats, ImageFormats, NULL);
//                     printf("  ---------------------------------\n");
//                     printf("  3D Image Formats Supported (%u)\n", uiNumSupportedFormats); 
//                     printf("  ---------------------------------\n");
//                     printf("  %-6s%-16s%-22s\n\n", "#", "Channel Order", "Channel Type"); 
//                     for(unsigned int i = 0; i < uiNumSupportedFormats; i++) 
//                     {  
//                         printf("  %-6u%-16s%-22s\n", (i + 1),
//                             oclImageFormatString(ImageFormats[i].image_channel_order), 
//                             oclImageFormatString(ImageFormats[i].image_channel_data_type));
//                     }
//                     printf("\n"); 
//                     delete [] ImageFormats;

// ================================================================================================

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
                    _texEntryPointsColor = new kisscl::GLTexture(_clContext, CL_MEM_READ_ONLY, entryPoints->getColorTexture()->getRepresentation<ImageRepresentationGL>()->getTexture());
                    delete _texExitPointsColor;
                    _texExitPointsColor = new kisscl::GLTexture(_clContext, CL_MEM_READ_ONLY, exitPoints->getColorTexture()->getRepresentation<ImageRepresentationGL>()->getTexture());
                    delete _texOutColor;


                    tgt::Texture* ttt = new tgt::Texture(0, dims, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                    ttt->uploadTexture();
                    ttt->setWrapping(tgt::Texture::CLAMP_TO_EDGE);
                    _texOutColor = new kisscl::GLTexture(_clContext, CL_MEM_WRITE_ONLY, ttt);


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

                    ImageData* id = new ImageData(2, dims, 4);
                    ImageRepresentationGL::create(id, ttt);
                    RenderData* rd = new RenderData();
                    rd->addColorTexture(id);
                    LGL_ERROR;

                    data.addData(_targetImageID.getValue(), rd);
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

        setValid();
    }

    std::string CLRaycaster::generateHeader() const {
        std::string toReturn;
        return toReturn;
    }

}
