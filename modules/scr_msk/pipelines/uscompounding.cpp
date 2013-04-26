// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#include "uscompounding.h"


namespace campvis {


    UsCompounding::UsCompounding() 
        : VisualizationPipeline()
        , p_camera("Camera", "Camera")
        , _reader()
        , _renderer(_canvasSize)
        , _trackballEH(0)
    {
        addProcessor(&_reader);
        addProcessor(&_renderer);

        addProperty(&p_camera);

        _trackballEH = new TrackballNavigationEventHandler(this, &p_camera, _canvasSize.getValue());
        _eventHandlers.push_back(_trackballEH);
    }

    UsCompounding::~UsCompounding() {
        delete _trackballEH;
    }

    void UsCompounding::init() {
        VisualizationPipeline::init();

        p_camera.addSharedProperty(&_renderer.p_camera);

        _reader.p_url.setValue("C:/Users/SchuCh01/Documents/Data/Ultrasound/2012-12-12-Test/9l4sweep/content.xml");
        _reader.p_targetImageID.connect(&_renderer.p_sourceImageID);
        _renderer.s_boundingBoxChanged.connect(this, &UsCompounding::onBoundingBoxChanged);
        _renderer.p_targetImageID.setValue("us.frame.output");
        _renderTargetID.setValue(_renderer.p_targetImageID.getValue());

        _trackballEH->setViewportSize(_effectiveRenderTargetSize.getValue());
        _effectiveRenderTargetSize.s_changed.connect<UsCompounding>(this, &UsCompounding::onRenderTargetSizeChanged);
    }

    void UsCompounding::deinit() {
        _effectiveRenderTargetSize.s_changed.disconnect(this);
        VisualizationPipeline::deinit();
    }

    const std::string UsCompounding::getName() const {
        return "UsCompounding";
    }

    void UsCompounding::onRenderTargetSizeChanged(const AbstractProperty* prop) {
        _trackballEH->setViewportSize(_canvasSize.getValue());
        float ratio = static_cast<float>(_effectiveRenderTargetSize.getValue().x) / static_cast<float>(_effectiveRenderTargetSize.getValue().y);
        p_camera.setWindowRatio(ratio);
    }

    void UsCompounding::onBoundingBoxChanged(tgt::Bounds b) {
        tgt::vec3 pos = b.center() - tgt::vec3(0, 0, tgt::length(b.diagonal()));

        _trackballEH->setSceneBounds(b);
        _trackballEH->setCenter(b.center());
        _trackballEH->reinitializeCamera(pos, b.center(), p_camera.getValue().getUpVector());
    }

}