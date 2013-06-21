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

#include "trackedussweepframerenderer3d.h"
#include "tgt/bounds.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationrendertarget.h"

#include "core/datastructures/meshgeometry.h"
#include "core/datastructures/facegeometry.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

#include "modules/scr_msk/datastructures/usinterfacedata.h"
#include <usinterface/include/trackedusfileio.h>
#include <usinterface/include/trackedusframe.h>
#include <usinterface/include/trackedussweep.h>
#include <usinterface/include/sweepvisitor/sweepsmoothinggaussian.h>


namespace {
    struct AverageTopCornersAccessFunc {
        vct3 operator() (const TrackedUSSweep::CornerUnion* corners, size_t index) {
            return (corners[index]._topLeftCorner + corners[index]._topRightCorner) * 0.5;
        }
    };

    template<class AccessFunc, int KERNEL_SIZE>
    struct GaussianSmoothingFunc {
        AccessFunc a;
        double kernel[KERNEL_SIZE + 1];
        double sigma;

        GaussianSmoothingFunc() {
            sigma = static_cast<double>(KERNEL_SIZE) / 2.5; // rough estimate

            for (size_t i = 0; i <= KERNEL_SIZE; ++i) {
                double f = static_cast<float>(i);
                kernel[i] = exp(-(f*f) / (2.0 * sigma * sigma));
            }
        }

        vct3 operator() (const TrackedUSSweep::CornerUnion* corners, size_t index, size_t start, size_t end) {
            vct3 toReturn(0.0);
            double norm = 0.0;
            for (int offset = -KERNEL_SIZE; offset <= KERNEL_SIZE; ++offset) {
                int i = static_cast<int>(index) + offset;
                if (i >= start && i < end) {
                    toReturn += a(corners, i) * kernel[abs(offset)];
                    norm += kernel[abs(offset)];
                }
            }

            toReturn /= norm;
            return toReturn;
        }
    };
}

namespace campvis {
    const std::string TrackedUsSweepFrameRenderer3D::loggerCat_ = "CAMPVis.modules.vis.TrackedUsSweepFrameRenderer3D";

    TrackedUsSweepFrameRenderer3D::TrackedUsSweepFrameRenderer3D(IVec2Property& canvasSize)
        : VisualizationProcessor(canvasSize)
        , p_sourceImageID("sourceFioID", "Input Tracked US File IO", "", DataNameProperty::READ, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_camera("Camera", "Camera")
        , p_sweepNumber("sweepNumber", "SweepNumber", 0, 0, 0, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES)
        , p_frameNumber("sliceNumber", "Slice Number", 0, 0, 0)
        , p_showConfidenceMap("ShowConfidenceMap", "Show Confidence Map", false)
        , p_smoothButton("SmoothButton", "Smooth Tracking")
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _shader(0)
        , _currentSweep(0)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_camera);
        addProperty(&p_sweepNumber);
        addProperty(&p_frameNumber);
        addProperty(&p_showConfidenceMap);
        addProperty(&p_smoothButton);
        addProperty(&p_transferFunction);

    }

    TrackedUsSweepFrameRenderer3D::~TrackedUsSweepFrameRenderer3D() {

    }

    void TrackedUsSweepFrameRenderer3D::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/scr_msk/glsl/trackedussweepframerenderer3d.frag", "", false);
        p_smoothButton.s_clicked.connect(this, &TrackedUsSweepFrameRenderer3D::onSmoothButtonClicked);
    }

    void TrackedUsSweepFrameRenderer3D::deinit() {
        p_smoothButton.s_clicked.disconnect(this);
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
        delete _currentSweep;
    }

    void TrackedUsSweepFrameRenderer3D::process(DataContainer& data) {
        DataContainer::ScopedTypedData< TrackedUsFileIoData > fio(data, p_sourceImageID.getValue());

        if (fio != 0) {
            if (hasInvalidProperties()) {
                updateProperties(const_cast<TrackedUSFileIO*>(fio->getData()));
            }

            if (_currentSweep != 0) {
                int frameNr = p_frameNumber.getValue();
                const tgt::Camera& cam = p_camera.getValue();

                // ultra ugly and dirty hacking:
                std::vector<tgt::vec3> corners(
                    reinterpret_cast<const tgt::dvec3*>(_currentSweep->getCorner(frameNr)._corners), 
                    reinterpret_cast<const tgt::dvec3*>(_currentSweep->getCorner(frameNr)._corners) + 4);

                std::vector<tgt::vec3> texCoords;
                texCoords.push_back(tgt::vec3(0.f, 1.f, 0.f)); // swapped top/bottom texture coordinates to
                texCoords.push_back(tgt::vec3(1.f, 1.f, 0.f)); // comply with mirrored y axis in OpenGL
                texCoords.push_back(tgt::vec3(1.f, 0.f, 0.f));
                texCoords.push_back(tgt::vec3(0.f, 0.f, 0.f));

                FaceGeometry slice(corners, texCoords);
                MeshGeometry bb = MeshGeometry::createCube(_bounds, tgt::Bounds(tgt::vec3(-1.f), tgt::vec3(-1.f)));

                const unsigned char* tmp = (p_showConfidenceMap.getValue() ? _currentSweep->getConfidenceMap(frameNr) : _currentSweep->getTrackedUSFrame(frameNr)->getImageBuffer());
                size_t width = (p_showConfidenceMap.getValue() ? _currentSweep->Width() / 4 : _currentSweep->Width());
                size_t height = (p_showConfidenceMap.getValue() ? _currentSweep->Height() / 4 : _currentSweep->Height());
                if (tmp != 0) {
                    std::pair<ImageData*, ImageRepresentationRenderTarget*> rt = ImageRepresentationRenderTarget::createWithImageData(_renderTargetSize.getValue());

                    glPushAttrib(GL_ALL_ATTRIB_BITS);
                    glEnable(GL_DEPTH_TEST);
                    _shader->activate();
                    _shader->setIgnoreUniformLocationError(true);
                    _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(_renderTargetSize.getValue()));
                    _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
                    _shader->setUniform("_viewMatrix", cam.getViewMatrix());

                    tgt::TextureUnit inputUnit, tfUnit;
                    inputUnit.activate();

                    tgt::Texture tex(
                        const_cast<unsigned char*>(tmp), 
                        tgt::vec3(width, height, 1), 
                        GL_ALPHA,
                        GL_ALPHA8,
                        GL_UNSIGNED_BYTE, 
                        tgt::Texture::LINEAR);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    tex.setType(GL_TEXTURE_2D);
                    tex.bind();
                    tex.uploadTexture();
                    tex.setWrapping(tgt::Texture::CLAMP);
                    tex.setPixelData(0);

                    _shader->setUniform("_texture", inputUnit.getUnitNumber());
                    _shader->setUniform("_textureParameters._size", tgt::vec2(_currentSweep->Width(), _currentSweep->Height()));
                    _shader->setUniform("_textureParameters._sizeRCP", tgt::vec2(1.f) / tgt::vec2(_currentSweep->Width(), _currentSweep->Height()));
                    _shader->setUniform("_textureParameters._numChannels", 1);

                    p_transferFunction.getTF()->bind(_shader, tfUnit);

                    rt.second->activate();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    _shader->setAttributeLocation(0, "in_Position");
                    _shader->setAttributeLocation(1, "in_TexCoord");

                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    bb.render(GL_LINE_LOOP);

                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    slice.render(GL_POLYGON);
                    rt.second->deactivate();

                    _shader->setIgnoreUniformLocationError(false);
                    _shader->deactivate();
                    tgt::TextureUnit::setZeroUnit();
                    glPopAttrib();

                    data.addData(p_targetImageID.getValue(), rt.first);
                    p_targetImageID.issueWrite();
                }
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    void TrackedUsSweepFrameRenderer3D::updateProperties(TrackedUSFileIO* fio) {
        if (fio != 0) {
            const TrackedUSFileXMLHeader::TrackedUSFileStudy *study = fio->getStudyHeader(0);
            p_sweepNumber.setMaxValue(static_cast<int>(study->Series.size()) - 1);

            int seriesno = p_sweepNumber.getValue();

            if (seriesno < study->Series.size()) {
                delete _currentSweep;
                fio->unmap();


                bool retval = fio->map(seriesno);
                if (retval) {
                    int totalFrames=study->Series[seriesno]->BytesAvailable/study->Series[seriesno]->FrameSize;
                    p_frameNumber.setMaxValue(totalFrames - 1);

                    _currentSweep = new TrackedUSSweep(study->Series[seriesno]->Width,
                        study->Series[seriesno]->Height, 
                        totalFrames, 
                        fio->getData(), 
                        study->Series[seriesno]->Tracking[0]->CalibrationMatrix(),
                        study->Series[seriesno]->Tracking[0]->RegistrationMatrix(),
                        fio->getConfidenceMap());

                    _currentSweep->SetPixelSize(study->Series[seriesno]->PixelSizeX, study->Series[seriesno]->PixelSizeY);
                    _currentSweep->CalculateCornersAndPose();
                    _currentSweep->CalculatePrincipalAxes();

                    if (_currentSweep->getConfidenceMap() == 0) {
                        p_showConfidenceMap.setValue(false);
                        p_showConfidenceMap.setVisible(false);
                    }
                    else {
                        p_showConfidenceMap.setVisible(true);
                    }

                    updateBoundingBox();
                }
                else {
                    _currentSweep = 0;
                    LERROR("Failed to map sweep " + seriesno);
                    return;
                }
            }
        }

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    const TrackedUSSweep* TrackedUsSweepFrameRenderer3D::getCurrentSweep() const {
        return _currentSweep;
    }

    void TrackedUsSweepFrameRenderer3D::onSmoothButtonClicked() {
        if (_currentSweep != 0) {
            SweepSmoothingGaussian smoother(_currentSweep, 16);
            smoother.applyToAll();
            updateBoundingBox();
            invalidate(AbstractProcessor::INVALID_RESULT);
        }
    }

    void TrackedUsSweepFrameRenderer3D::updateBoundingBox() {
        tgt::dvec3 llf, urb;
        _currentSweep->BoundingBox(llf.elem, urb.elem);
        _bounds = tgt::Bounds(llf, urb);
        s_boundingBoxChanged(_bounds);
    }

}
