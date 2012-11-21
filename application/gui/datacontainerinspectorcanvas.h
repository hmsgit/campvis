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

#ifndef DATACONTAINERINSPECTORCANVAS_H__
#define DATACONTAINERINSPECTORCANVAS_H__

#include "sigslot/sigslot.h"
#include "tgt/painter.h"
#include "tgt/event/eventlistener.h"
#include "tgt/event/mouseevent.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tbb/include/tbb/mutex.h"

#include "core/tools/opengljobprocessor.h"

namespace tgt {
    class Shader;
    class Texture;
    class TextureUnit;
}

namespace campvis {
    class AbstractPipeline;
    class DataContainer;
    class DataContainerTreeWidget;
    class DataHandle;
    class FaceGeometry;

    class DataContainerInspectorCanvas : public tgt::QtThreadedCanvas, tgt::Painter, public tgt::EventListener, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerInspectorCanvas.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        DataContainerInspectorCanvas(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~DataContainerInspectorCanvas();

        /**
         * Initializes the OpenGL stuff (e.g. shaders).
         * Must be called with a valid and locked OpenGL context.
         */
        virtual void init();

        /**
         * Deinitializes the OpenGL stuff (e.g. shaders).
         * Must be called with a valid and locked OpenGL context.
         */
        void deinit();

        /**
         * Set the DataContainer this widget is inspecting.
         * \param   dataContainer   The DataContainer this widget shall inspect, may be 0.
         */
        void setDataContainer(DataContainer* dataContainer);

        /**
         * Size hint for the default window size
         * \return QSize(640, 480)
         */
        QSize sizeHint() const;

        /**
         * Performs the painting.
         */
        void paint();

        /// This is meant be overridden to adjust camera settings to new canvas dimensions
        virtual void sizeChanged(const tgt::ivec2&);

        /**
         * Called on double click event on this canvas
         * \param   e   Mouse event arguments
         */
        virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);

    private slots:

    protected:
        /**
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * If \a name equals the name of the renderTarget, the s_renderTargetChanged signal will be emitted.
         * \param   name    Name of the added data.
         * \param   dh      DataHandle to the newly added data.
         */
        void onDataContainerDataAdded(const std::string& name, const DataHandle& dh);

        /**
         * To be called when the canvas is invalidated, issues new paint job.
         */
        void invalidate();

        /**
         * Renders the given texture.
         * Binds the texture to the shader, sets the uniforms and renders the quad.
         * \param   texture     The texture to render.
         */
        void paintTexture(const tgt::Texture* texture);

        /**
         * Creates the quad used for rendering the textures.
         */
        void createQuad();

        DataContainer* _dataContainer;                  ///< The DataContainer this widget is inspecting
        std::map<std::string, DataHandle> _handles;     ///< Local copy of the DataHandles to inspect
        tbb::mutex _localMutex;                         ///< Mutex protecting the local members

        tgt::Shader* _paintShader;                      ///< GLSL shader for rendering the textures
        FaceGeometry* _quad;                            ///< Quad used for rendering

        tgt::ivec2 _numTiles;                           ///< number of tiles on texture overview
        tgt::ivec2 _quadSize;                           ///< size in pixels for each tile in overview
        size_t _selectedTexture;                        ///< index of selected texture for fullscreen view
        bool _renderFullscreen;                         ///< flag whether to render in full screen

    };
}

#endif // DATACONTAINERINSPECTORCANVAS_H__
