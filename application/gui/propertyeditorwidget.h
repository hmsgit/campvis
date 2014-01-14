// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef PROPERTYEDITORWIDGET_H_
#define PROPERTYEDITORWIDGET_H_

#include "sigslot/sigslot.h"
#include "tgt/painter.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tbb/mutex.h"

#include "application/gui/qtdatahandle.h"
#include "application/gui/datacontainerinspectorcanvas.h"
#include "application/gui/properties/propertycollectionwidget.h"
#include "application/gui/properties/stringpropertywidget.h"
#include "core/tools/opengljobprocessor.h"
#include "modules/io/processors/genericimagereader.h"
#include "application/gui/datacontainerinspectorwidget.h"

#include <QLabel>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QPushButton>
#include <QMetaObject>

class QModelIndex;
class QItemSelection;

namespace tgt {
    class Texture;
    class TextureUnit;
    class Shader;
}

namespace campvis {

    class PropertyEditorWidget : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerInspectorWidget.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit PropertyEditorWidget(DataContainerInspectorWidget* treeModel, QWidget* parent = nullptr);

        /**
         * Destructor.
         */
        ~PropertyEditorWidget();

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
         * Updates color of the info widget
         */
        void updateColor();

        /**
         * Updates depth of the info widget
         */
        void updateDepth();

    signals:

    private slots:
        /**
         * Slot being called when the user clicks on the "Cancel" button.
         */
        void onBtnCancelClicked();

        /**
         * Slot being called when the user clicks on the "Load File" button.
         */
        void onBtnLoadFileClicked();

    protected:
        /**
         * Setup the GUI stuff
         */
        void setupGUI();

    protected:

        bool _inited;
        DataContainer* _dataContainer;                  ///< The DataContainer this widget is inspecting
        DataContainerInspectorCanvas* _canvas;          ///< The OpenGL canvas for rendering the DataContainer's contents
        QVBoxLayout* _mainLayout;                       ///< Layout for this widget
        QVBoxLayout* rightLayout;                       ///< Layout for the _infoWidget
        QScrollArea* _pipelinePropertiesScrollArea;     ///< Scroll area for _pipelinePropertiesWidget

    protected:
        QPushButton* _btnCancel;
        QPushButton* _btnLoadFile;
        StringProperty _fileName;
        PropertyCollectionWidget* _propCollectionWidget;    ///< Widget for browsing the PropertyCollection of the selected pipeline/processor

        GenericImageReader *_imgReader;
        DataContainerInspectorWidget* _parent;
        
        static const std::string loggerCat_;

    private:

    };
}

#endif // PROPERTYEDITORWIDGET_H_
