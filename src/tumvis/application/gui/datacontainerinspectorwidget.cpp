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

#include "datacontainerinspectorwidget.h"

#include "tgt/assert.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/datahandle.h"
#include "application/gui/datacontainertreewidget.h"

namespace TUMVis {

    DataContainerInspectorWidget::DataContainerInspectorWidget(QWidget* parent) 
        : QWidget(parent)
        , _dataContainer(0)
        , _selectedDataHandle(0)
        , _dctWidget(0)
        , _canvas(0)
        , _mainLayout(0)
        , _infoWidget(0)
        , _infoWidgetLayout(0)
    {
        setupGUI();
    }

    DataContainerInspectorWidget::~DataContainerInspectorWidget() {
        delete _selectedDataHandle;
    }

    void DataContainerInspectorWidget::setDataContainer(DataContainer* dataContainer) {
        if (_dataContainer != 0) {
            _dataContainer->s_changed.disconnect(this);
        }

        _dataContainer = dataContainer;
        _dctWidget->update(dataContainer);
        
        if (_dataContainer != 0) {
            _dataContainer->s_changed.connect(this, &DataContainerInspectorWidget::onDataContainerChanged);
        }
    }

    void DataContainerInspectorWidget::onDataContainerChanged() {
    }

    QSize DataContainerInspectorWidget::sizeHint() const {
        return QSize(640, 480);
    }

    void DataContainerInspectorWidget::onDCTWidgetItemClicked(const QModelIndex& index) {
        if (index.isValid()) {
            // Yak, this is so ugly - another reason why GUI programming sucks...
            QVariant item = index.data(Qt::UserRole);

            delete _selectedDataHandle;
            _selectedDataHandle = new DataHandle(*static_cast<DataHandle*>(item.value<void*>()));

            QModelIndex idxName = index.sibling(index.row(), 0);
            _selectedDataHandleName = idxName.data(Qt::DisplayRole).toString();
        }
        else {
            delete _selectedDataHandle;
            _selectedDataHandle = 0;
            _selectedDataHandleName = "";
        }

        updateInfoWidget();
    }

    void DataContainerInspectorWidget::setupGUI() {
        _mainLayout = new QHBoxLayout();
        _mainLayout->setSpacing(4);
        setLayout(_mainLayout);

        _dctWidget = new DataContainerTreeWidget(this);
        _mainLayout->addWidget(_dctWidget);

        _infoWidget = new QWidget(this);
        _infoWidgetLayout = new QVBoxLayout();
        _infoWidgetLayout->setSpacing(4);
        _infoWidget->setLayout(_infoWidgetLayout);

        _lblName = new QLabel(QString("Name: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblName);

        _lblTimestamp = new QLabel("Timestamp: ", _infoWidget);
        _infoWidgetLayout->addWidget(_lblTimestamp);

        _canvas = CtxtMgr.createContext("DataContainerInspector", "", tgt::ivec2(128, 128), tgt::GLCanvas::RGBA, _infoWidget);
        _infoWidgetLayout->addWidget(_canvas, 1);

        _mainLayout->addWidget(_infoWidget, 1);

        connect(
            _dctWidget, SIGNAL(clicked(const QModelIndex&)), 
            this, SLOT(onDCTWidgetItemClicked(const QModelIndex&)));
    }

    void DataContainerInspectorWidget::updateInfoWidget() {
        if (_selectedDataHandle != 0) {
            _lblTimestamp->setText("Timestamp: " + QString::number(_selectedDataHandle->getTimestamp()));
            _lblName->setText("Name: " + _selectedDataHandleName);
        }
        else {
            _lblTimestamp->setText("Timestamp: ");
            _lblName->setText("Name: ");
        }
    }

}