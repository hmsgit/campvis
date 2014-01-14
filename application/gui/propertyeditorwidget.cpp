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

#include "propertyeditorwidget.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/filesystem.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"

#include "core/datastructures/abstractdata.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#ifdef CAMPVIS_HAS_MODULE_COLUMBIA
#include "modules/columbia/datastructures/fiberdata.h"
#endif

#include "application/gui/datacontainerinspectorwidget.h"
#include "application/gui/datacontainertreewidget.h"
#include "application/gui/qtdatahandle.h"
#include "modules/io/processors/genericimagereader.h"

#include <QFileDialog>

namespace campvis {

    const std::string PropertyEditorWidget::loggerCat_ = "CAMPVis.application.PropertyEditorWidget";

    PropertyEditorWidget::PropertyEditorWidget(DataContainerInspectorWidget* treeModel, QWidget* parent) 
        : QWidget(parent)
        , _inited(false)
        , _dataContainer(0)
        //, _dctWidget(0)
        , _canvas(0)
        //, _pcWidget(0)
        , _mainLayout(0)
        //, _infoWidget(0)
        , _pipelinePropertiesScrollArea(0)
        , _propCollectionWidget(0)
        //, _infoWidgetLayout(0)
        , _fileName("fileName", "Image URL", "")
    {
        _parent = treeModel;
        setupGUI();
    }

    PropertyEditorWidget::~PropertyEditorWidget() {
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }
    }

    void PropertyEditorWidget::setDataContainer(DataContainer* dataContainer) {
        if (_dataContainer != 0) {
            //_dataContainer->s_dataAdded.disconnect(this);
        }

        _dataContainer = dataContainer;
        //_dctWidget->update(dataContainer);
        //updateInfoWidget();

        if (_dataContainer != 0) {
            //_dataContainer->s_dataAdded.connect(this, &PropertyEditorWidget::onDataContainerDataAdded);
        }
    }


    void PropertyEditorWidget::setParentx(DataContainerInspectorWidget* parent) {
        this->_parent = parent;
    }
    void PropertyEditorWidget::setImageReader (GenericImageReader* imgReader) {
        this->_imgReader = imgReader;
    }
    void PropertyEditorWidget::updatePropCollection () {
        this->_propCollectionWidget->updatePropCollection(this->_imgReader, this->_dataContainer);
    }


    void PropertyEditorWidget::onDataContainerDataAdded(const std::string& key, const DataHandle& dh) {
        // copy QtDataHandle because signal will be handled by a different thread an indefinite amount of time later:
        emit dataContainerChanged();
    }

    QSize PropertyEditorWidget::sizeHint() const {
        return QSize(200, 300);
    }


    void PropertyEditorWidget::setupGUI() {
        setWindowTitle(tr("Select Property Value"));

        _mainLayout = new QVBoxLayout();
        _mainLayout->setSpacing(4);
        setLayout(_mainLayout);

        _pipelinePropertiesScrollArea = new QScrollArea(this);
        _pipelinePropertiesScrollArea->setWidgetResizable(true);
        _pipelinePropertiesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        _pipelinePropertiesScrollArea->setFrameStyle(QScrollArea::NoFrame);
        
        this->installEventFilter(this);

//         _btnBrowse = new StringPropertyWidget(&_fileName, this);
//         _mainLayout->addWidget(_btnBrowse);
        
        _propCollectionWidget = new PropertyCollectionWidget(_pipelinePropertiesScrollArea);
        _mainLayout->addWidget(_propCollectionWidget);
        //_mainLayout->addStretch();

        _btnLoadFile = new QPushButton(tr("Load File"), this);
        _mainLayout->addWidget(_btnLoadFile);
        _btnCancel = new QPushButton(tr("Cancel"), this);
        _mainLayout->addWidget(_btnCancel);

        

        qRegisterMetaType<QtDataHandle>("QtDataHandle");
        connect(
            _btnCancel, SIGNAL(clicked()),
            this, SLOT(onBtnCancelClicked()));
        connect(
            _btnLoadFile, SIGNAL(clicked()),
            this, SLOT(onBtnLoadFileClicked()));
        connect(
            this, SIGNAL(dataContainerChanged()),
            this->_parent, SLOT(onDataContainerChanged(const QString&, QtDataHandle)));
/*

        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.connect(this, &PropertyEditorWidget::updateDataInspector);
        }*/

        //_fileName.addSharedProperty(&_imgReader->p_url);
    }

    void PropertyEditorWidget::updateColor(){
        const tgt::Color color = _canvas->getCapturedColor();
    }

    void PropertyEditorWidget::updateDepth(){

        float depth = _canvas->getCapturedDepth();
    }

    //QString PropertyEditorWidget::humanizeBytes(size_t numBytes) const {
    //    QString units[5] = { tr(" Bytes"), tr(" KB"), tr(" MB"), tr(" GB"), tr(" TB") };
    //    size_t index = 0;
    //    size_t remainder = 0;

    //    while (numBytes > 1024 && index < 4) {
    //        remainder = numBytes % 1024;
    //        numBytes /= 1024;
    //        ++index;
    //    }

    //    if (remainder != 0)
    //        return QString::number(numBytes) + "." + QString::number(remainder) + units[index];
    //    else
    //        return QString::number(numBytes) + units[index];
    //}

    void PropertyEditorWidget::init() {
        //if (_canvas != 0)
            //_canvas->init(this);

        _inited = true;
    }

    void PropertyEditorWidget::deinit() {
        _inited = false;
        //if (_canvas != 0)
            //_canvas->deinit();

        //_pcWidget->updatePropCollection(0, 0);

        if (_dataContainer != 0) {
            //_dataContainer->s_dataAdded.disconnect(this);
        }

        //_dataContainer = 0;
        //_dctWidget->update(0);
    }

    void PropertyEditorWidget::onBtnCancelClicked() {
        this->close();

    }

    void PropertyEditorWidget::updateDataInspector(const std::string& key, const DataHandle&dh) {
        //emit dataContainerChanged(QString::fromStdString(key), QtDataHandle(dh));
    }

    void PropertyEditorWidget::onBtnLoadFileClicked() {
        _imgReader->process(*_dataContainer);
        //emit dataContainerChanged();
        this->_parent->setDataContainer(_dataContainer);
        this->close();
    }

}