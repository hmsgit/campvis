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

#include "datacontainerfileloaderwidget.h"

namespace campvis {

    const std::string DataContainerFileLoaderWidget::loggerCat_ = "CAMPVis.application.DataContainerFileLoaderWidget";

    DataContainerFileLoaderWidget::DataContainerFileLoaderWidget(DataContainerInspectorWidget* parentDataInspector, QWidget* parent) 
        : QWidget(parent)
        , _dataContainer(0)
        , _layout(0)
        , _pipelinePropertiesScrollArea(0)
        , _propCollectionWidget(0)
        , _fileName("fileName", "Image URL", "")
    {
        this->_parent = parentDataInspector;
        this->_dataContainer = this->_parent->getDataContainer();
        this->_imgReader = new GenericImageReader();
        this->_imgReader->setVisibibility(".mhd", true);
        setupGUI();
    }

    DataContainerFileLoaderWidget::~DataContainerFileLoaderWidget() {
        if (this->_dataContainer != 0) {
            this->_dataContainer->s_dataAdded.disconnect(this);
        }
    }

    void DataContainerFileLoaderWidget::setDataContainer(DataContainer* dataContainer) {
        this->_dataContainer = dataContainer;
    }

    QSize DataContainerFileLoaderWidget::sizeHint() const {
        return QSize(300, 350);
    }


    void DataContainerFileLoaderWidget::setupGUI() {
        setWindowTitle(tr("Browse File"));

        this->_layout = new QGridLayout();
        this->_layout->setSpacing(2);
        setLayout(_layout);

        this->_pipelinePropertiesScrollArea = new QScrollArea(this);
        this->_pipelinePropertiesScrollArea->setWidgetResizable(true);
        this->_pipelinePropertiesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->_pipelinePropertiesScrollArea->setFrameStyle(QScrollArea::NoFrame);
        
        this->installEventFilter(this);

        this->_propCollectionWidget = new PropertyCollectionWidget(this->_pipelinePropertiesScrollArea);
        this->_layout->addWidget(this->_propCollectionWidget, 0, 0, 1, 2);
        this->_propCollectionWidget->updatePropCollection(this->_imgReader, this->_dataContainer);

        this->_btnLoadFile = new QPushButton(tr("Load"), this);
        this->_layout->addWidget(this->_btnLoadFile, 1, 0, 1, 1);
        this->_btnCancel = new QPushButton(tr("Cancel"), this);
        this->_layout->addWidget(this->_btnCancel, 1, 1, 1, 1);

        qRegisterMetaType<QtDataHandle>("QtDataHandle");
        connect(
            this->_btnCancel, SIGNAL(clicked()),
            this, SLOT(onBtnCancelClicked()));
        connect(
            this->_btnLoadFile, SIGNAL(clicked()),
            this, SLOT(onBtnLoadFileClicked()));
    }

    void DataContainerFileLoaderWidget::init() {
    }

    void DataContainerFileLoaderWidget::deinit() {
        delete _imgReader;
        this->_imgReader = nullptr;
    }

    void DataContainerFileLoaderWidget::onBtnCancelClicked() {
        delete _imgReader;
        this->_imgReader = nullptr;
        this->close();
    }

    void DataContainerFileLoaderWidget::onBtnLoadFileClicked() {
        this->_imgReader->process(*_dataContainer);
        this->_parent->setDataContainer(_dataContainer);
        this->close();
    }

}