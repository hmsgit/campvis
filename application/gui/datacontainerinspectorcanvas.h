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
#include "tbb/mutex.h"

#include "application/gui/qtdatahandle.h"

#include "core/properties/propertycollection.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/tools/opengljobprocessor.h"
#include "modules/vis/processors/geometryrenderer.h"

#include "core/eventhandlers/trackballnavigationeventlistener.h"
#include "core/datastructures/meshgeometry.h"

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
    class DataContainerInspectorWidget;

    /**
     * Stores information about the textures which store the rendered geomtery.
     * Note: The object's destroy() function should be called before deleting or releasing the object's memory.
     */
    struct GeometryTextureInfo {
        campvis::QtDataHandle _geomData;
        tgt::Texture* _texture;
        int _trackballIndx;
        
        GeometryTextureInfo(): _geomData(0), _texture(0){
        }
        
        void destroy()
        {
            delete _texture;
        }

        ~GeometryTextureInfo() {
        }
    };

    class DataContainerInspectorCanvas : public tgt::QtThreadedCanvas, tgt::Painter, public tgt::EventListener, public HasPropertyCollection {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerInspectorCanvas.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit DataContainerInspectorCanvas(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~DataContainerInspectorCanvas();

        /**
         * Initializes the OpenGL stuff (e.g. shaders).
         * Must be called with a valid and locked OpenGL context.
         */
        virtual void init(DataContainerInspectorWidget* _pWidget);

        /**
         * Deinitializes the OpenGL stuff (e.g. shaders).
         * Must be called with a valid and locked OpenGL context.
         */
        void deinit();

        /**
         * Reset the content of the canvas.
         * It will clear the array of textures, rendered geomteries, color buffers, depth buffers and its content.
         */
        void resetContent();

        void setDataHandles(const std::vector< std::pair<QString, QtDataHandle> >& handles);
        
        /**
         * returns the color value which is captured with the mouse.
         */
        const tgt::Color& getCapturedColor();

        /**
         * returns the depth value which is captured with the mouse.
         */
        const float& getCapturedDepth();

        /**
         * Size hint for the default window size
         * \return QSize(640, 480)
         */
        QSize sizeHint() const;

        /**
         * Schedule a repaint job for the inspector's render target
         */
        void repaint();

        /// This is meant be overridden to adjust camera settings to new canvas dimensions
        virtual void sizeChanged(const tgt::ivec2&);

        /**
         * Called on double click event on this canvas
         * \param   e   Mouse event arguments
         */
        virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);

        /**
         * Called on mouse move event on this canvas
         * \param   e   Mouse event arguments
         */
        virtual void mouseMoveEvent(tgt::MouseEvent* e);

        /**
         * Called on mouse wheel event on this canvas.
         * \param   e   Mouse event arguments
         */
        virtual void wheelEvent(tgt::MouseEvent* e);
        
        /**
         * Called on mouse press button event on this canvas.
         * \param   e   Mouse event arguments
         */
        virtual void mousePressEvent(tgt::MouseEvent* e);
        
        /**
         * Called on mouse release button event on this canvas.
         * \param   e   Mouse event arguments
         */
        virtual void mouseReleaseEvent(tgt::MouseEvent* e);
        
        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

        IntProperty p_currentSlice;
        IVec4Property p_meshSolidColor;                  ///< Color used to render the mesh object
        TransferFunctionProperty p_transferFunction;     ///< Transfer function

    private slots:
        /**
         * Slot being called when a QtDataHandle has been added to the DataContainer.
         * \param   key     Name of the QtDataHandle
         * \param   dh      The added QtDataHandle
         */
        void onDataContainerChanged(const QString& key, QtDataHandle dh);

    protected:
        /**
         * Performs the painting.
         */
        virtual void paint();

        /**
         * Performs the painting.
         */
        virtual void paintMeshGeomTextures();

        /**
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * If \a name equals the name of the renderTarget, the s_renderTargetChanged signal will be emitted.
         * \param   name    Name of the added data.
         * \param   dh      DataHandle to the newly added data.
         */
        void onDataContainerDataAdded(const std::string& name, const DataHandle& dh);

        /**
         * Updates the textures vector.
         * \note Only call with acquired lock and valid OpenGL context!!
         */
        void updateTextures();

        /**
         * Updates the textures vector elements that belongs to mesh geomteries.
         * \note Only call with acquired lock and valid OpenGL context!!
         */
        void updateMeshGeomRenderedTextures();

        /**
         * To be called when the canvas is invalidated, issues new paint job.
         */
        void invalidate();

        /**
         * To be called when the mesh geometry parts of the scene is invalidated, issues new paint job only for textures that belong to mesh geomteries.
         */
        void invalidateMeshGeomTextures();

        /**
         * Renders the given 2D texture.
         * Binds the texture to the shader, sets the uniforms and renders the quad.
         * \param   texture     The texture to render.
         * \param   uint2d      The texture unit that will be attached to rendering pipeline (2D for 2D textures).
         * \param   uint2d      The texture unit that will be attached to rendering pipeline (3D for 3D textures).
         */
        void paintTexture(const tgt::Texture* texture, const tgt::TextureUnit& unit2d, const tgt::TextureUnit& unit3d);

        /**
         * Renders the Meshgeometry into the geometry renderer color buffer and depth buffer.
         * Binds the texture to the shader, sets the uniforms and renders the mesh geometry.
         * \param   meshgeometry        The mesh to be rendered.
         * \param   colorBuffer         The color buffer that the object will be rendered to.
         * \param   depthBuffer         The depth buffer that the object will be rendered to.
         * \param   meshIndex           The index of the rendered mesh in the texture array.
         */
        void drawGeomteryData(const campvis::GeometryData* meshgeometry, tgt::Texture* colorBuffer, const int& trackballIndx);

        /**
         * Creates the quad used for rendering the textures.
         */
        void createQuad();

        std::map<QString, QtDataHandle> _handles;
        std::vector<const tgt::Texture*> _textures;     ///< vector of textures
        bool _texturesDirty;                            ///< Flag that shows that the textures need update or not.
        bool _meshGeomTexturesDirty;                    ///< Flag that shows that the mesh geometry textures need updare or not.

        DataContainer* _dataContainer;              ///< The DataContainer this widget is inspecting
        tbb::mutex _localMutex;                     ///< Mutex protecting the local members

        tgt::Shader* _paintShader;                  ///< GLSL shader for rendering the textures
        FaceGeometry* _quad;                        ///< Quad used for rendering

        tgt::Color _color;                          ///< Color under the mouse cursor
        float      _depth;							///< Depth under the mouse cursor
        DataContainerInspectorWidget* _widget;      ///< Pointer to the widget which has access to this canvas

        tgt::ivec2 _numTiles;                       ///< number of tiles on texture overview
        tgt::ivec2 _quadSize;                       ///< size in pixels for each tile in overview
        size_t _selectedTexture;                    ///< index of selected texture by mouse
        int _selectedTrackball;						///< index of selected trackball which will be updated currently
        bool _renderFullscreen;                     ///< flag whether to render in full screen

        int _currentSlice;							///< current slice if rendering a 3D image fullscreen, render MIP if negative

        tgt::Shader* _geometryRenderingShader;			                    ///< GLSL shader for rendering the mesh geomteries
        tgt::FramebufferObject* _frameBuffer;
        tgt::Texture* _depthBuffer;

        std::vector<campvis::CameraProperty*> _trackballCameraProperties;   ///< The property of the trackball camera. Used to pass the trackball camera to the shader.
        std::vector<TrackballNavigationEventListener*> _trackballEHs;       ///< TrackBall Event Handler for the camera rotating around the object in the canvas
        std::vector<GeometryTextureInfo> _geomTextureInfos;                 ///< Array of data regarding the rendered geomteries
    };
}

#endif // DATACONTAINERINSPECTORCANVAS_H__