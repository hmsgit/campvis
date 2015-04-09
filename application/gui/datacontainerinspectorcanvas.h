// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef DATACONTAINERINSPECTORCANVAS_H__
#define DATACONTAINERINSPECTORCANVAS_H__

#include "sigslot/sigslot.h"
#include "cgt/painter.h"
#include "cgt/event/eventlistener.h"
#include "cgt/event/mouseevent.h"
#include "cgt/glcontextmanager.h"
#include "cgt/qt/qtthreadedcanvas.h"
#include "tbb/mutex.h"

#include "application/gui/qtdatahandle.h"

#include "core/properties/metaproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/propertycollection.h"
#include "core/properties/transferfunctionproperty.h"

#include "modules/base/processors/lightsourceprovider.h"
#include "modules/base/processors/trackballcameraprovider.h"
#include "modules/vis/processors/geometryrenderer.h"

#include <memory>


namespace cgt {
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
    class GeometryData;
    class DataContainerInspectorWidget;

    class DataContainerInspectorCanvas : public cgt::QtThreadedCanvas, cgt::Painter, public cgt::EventListener, public HasPropertyCollection {
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
        virtual void init();

        /**
         * Deinitializes the OpenGL stuff (e.g. shaders).
         * Must be called with a valid and locked OpenGL context.
         */
        void deinit();

        /**
         * Set the DataHandles to show in the canvas to those in \a handles.
         * \param   handles vector of name-DataHandle pairs to show in the canvas.
         */
        void setDataHandles(const std::vector< std::pair<QString, QtDataHandle> >& handles);

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
        virtual void sizeChanged(const cgt::ivec2&);

        /**
         * Called on mouse move event on this canvas
         * \param   e   Mouse event arguments
         */
        virtual void mouseMoveEvent(cgt::MouseEvent* e);

        virtual void onEvent(cgt::Event* e);
        
        IntProperty p_currentSlice;                     ///< Currently selected slice to show
        TransferFunctionProperty p_transferFunction;    ///< Transfer function

        BoolProperty p_renderRChannel;                  ///< Flag whether to render Red channel
        BoolProperty p_renderGChannel;                  ///< Flag whether to render Green channel
        BoolProperty p_renderBChannel;                  ///< Flag whether to render Blue channel
        BoolProperty p_renderAChannel;                  ///< Flag whether to render Alpha channel

        MetaProperty p_geometryRendererProperties;      ///< MetaProperty for the GeometryRenderer

    signals:
        void s_colorChanged(const cgt::vec4&);
        void s_depthChanged(float depth);

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
         * Gets called when the data collection of this pipeline has changed and thus has notified its observers.
         * If \a name equals the name of the renderTarget, the s_renderTargetChanged signal will be emitted.
         * \param   name    Name of the added data.
         * \param   dh      DataHandle to the newly added data.
         */
        void onDataContainerDataAdded(const std::string& name, DataHandle dh);

        /**
         * Slot getting called when one of the observed properties changed and notifies its observers.
         * \param   prop    Property that emitted the signal
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);
        virtual void onGeometryRendererPropertyChanged(const AbstractProperty* prop);

        /**
         * Updates the textures vector.
         * \note Only call with acquired lock and valid OpenGL context!!
         */
        void updateTextures();

        /**
         * Resets the trackball and reinitializes the camera to the currently loaded geometries if necessary.
         */
        void resetTrackball();

        /**
         * To be called when the canvas is invalidated, issues new paint job.
         */
        void invalidate();

        /**
         * Renders the given 2D texture.
         * Binds the texture to the shader, sets the uniforms and renders the quad.
         * \param   texture     The texture to render. Must not be 0.
         * \param   uint2d      The texture unit that will be attached to rendering pipeline (2D for 2D textures).
         * \param   uint2d      The texture unit that will be attached to rendering pipeline (3D for 3D textures).
         */
        void paintTexture(const cgt::Texture* texture, const cgt::TextureUnit& unit2d, const cgt::TextureUnit& unit3d);

        /**
         * Renders \a geometry into a texture.
         * 
         * \param name 
         * \param geometry
         */
        void renderGeometryIntoTexture(const std::string& name, int textureIndex = -1);

        /**
         * Creates the quad used for rendering the textures.
         */
        void createQuad();

        
        std::map<QString, QtDataHandle> _handles;   ///< Map of DataHandles to show

        /// Vector of textures to render. Each DataHandle contains an ImageData that has an OpenGL representation.
        /// This ensures thread safety.
        std::vector<QtDataHandle> _textures;

        /// List of the names of all rendered geometries. This simplifies to update their rendering.
        std::vector< std::pair<std::string, int> > _geometryNames;

        bool _texturesDirty;                        ///< Flag that shows that the textures need update or not.
        bool _geometriesDirty;                      ///< Flag that shows that the rendered geometries need update or not.

        DataContainer* _dataContainer;              ///< The DataContainer this widget is inspecting
        tbb::mutex _localMutex;                     ///< Mutex protecting the local members

        cgt::Shader* _paintShader;                  ///< GLSL shader for rendering the textures
        std::unique_ptr<FaceGeometry> _quad;        ///< Quad used for rendering

        cgt::ivec2 _numTiles;                       ///< number of tiles on texture overview
        cgt::ivec2 _quadSize;                       ///< size in pixels for each tile in overview

        DataContainer _localDataContainer;          ///< Local DataContainer the GeometryRenderer works on
        IVec2Property p_viewportSize;

        TrackballCameraProvider _tcp;               ///< TrackballCameraProvider to do camera stuff
        GeometryRenderer _geometryRenderer;         ///< GeometryRenderer used to render geometries
    };
}

#endif // DATACONTAINERINSPECTORCANVAS_H__