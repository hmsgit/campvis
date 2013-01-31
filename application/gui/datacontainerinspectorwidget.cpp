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

#include "datacontainerinspectorwidget.h"

#include "tgt/assert.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"

#include "core/datastructures/abstractdata.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/imagerepresentationgl.h"

#include "application/gui/datacontainertreewidget.h"
#include "application/gui/qtdatahandle.h"

namespace campvis {

    DataContainerInspectorWidget::DataContainerInspectorWidget(QWidget* parent) 
        : QWidget(parent)
        , _quad(0)
        , _paintShader(0)
        , _currentSlice(-1)
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
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }
    }

    void DataContainerInspectorWidget::setDataContainer(DataContainer* dataContainer) {
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.disconnect(this);
        }

        _dataContainer = dataContainer;
        _dctWidget->update(dataContainer);

        if (_canvas != 0)
            _canvas->setDataContainer(_dataContainer);
        
        if (_dataContainer != 0) {
            _dataContainer->s_dataAdded.connect(this, &DataContainerInspectorWidget::onDataContainerDataAdded);
        }
    }

    void DataContainerInspectorWidget::onDataContainerDataAdded(const std::string& key, const DataHandle& dh) {
        // copy QtDataHandle because signal will be handled by a different thread an indefinite amount of time later:
        emit dataContainerChanged(QString::fromStdString(key), QtDataHandle(dh));
    }

    QSize DataContainerInspectorWidget::sizeHint() const {
        return QSize(640, 480);
    }

    void DataContainerInspectorWidget::onDCTWidgetItemClicked(const QModelIndex& index) {
        if (index.isValid()) {
            // Yak, this is so ugly - another reason why GUI programming sucks...
            QVariant item = index.data(Qt::UserRole);

            _selectedDataHandle = item.value<QtDataHandle>();

            QModelIndex idxName = index.sibling(index.row(), 0);
            _selectedDataHandleName = idxName.data(Qt::DisplayRole).toString();

            _selectedIndex = tgt::ivec2(index.row(), index.column());
        }
        else {
            _selectedDataHandle = QtDataHandle(0);
            _selectedDataHandleName = "";
            _selectedIndex = tgt::ivec2(-1, -1);
        }

        updateInfoWidget();
    }

    void DataContainerInspectorWidget::onDCTWidgetDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
        if (topLeft.row() <= _selectedIndex.x && topLeft.column() <= _selectedIndex.y && bottomRight.row() >= _selectedIndex.x && bottomRight.column() >= _selectedIndex.y) {
            onDCTWidgetItemClicked(topLeft.sibling(_selectedIndex.x, _selectedIndex.y));
        }
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

        _lblLocalMemoryFootprint = new QLabel(QString("Local Memory Footprint: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblLocalMemoryFootprint);

        _lblVideoMemoryFootprint = new QLabel(QString("Video Memory Footprint: "), _infoWidget);
        _infoWidgetLayout->addWidget(_lblVideoMemoryFootprint);

        _lblTimestamp = new QLabel("Timestamp: ", _infoWidget);
        _infoWidgetLayout->addWidget(_lblTimestamp);

        _canvas = new DataContainerInspectorCanvas(_infoWidget);
        _infoWidgetLayout->addWidget(_canvas, 1);

        _mainLayout->addWidget(_infoWidget, 1);

        qRegisterMetaType<QtDataHandle>("QtDataHandle");
        connect(
            _dctWidget, SIGNAL(clicked(const QModelIndex&)), 
            this, SLOT(onDCTWidgetItemClicked(const QModelIndex&)));
        connect(
            _dctWidget->getTreeModel(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), 
            this, SLOT(onDCTWidgetDataChanged(const QModelIndex&, const QModelIndex&)));
        connect(
            this, SIGNAL(dataContainerChanged(const QString&, QtDataHandle)),
            _dctWidget->getTreeModel(), SLOT(onDataContainerChanged(const QString&, QtDataHandle)));
    }

    void DataContainerInspectorWidget::updateInfoWidget() {
        if (_selectedDataHandle.getData() != 0) {
            _lblTimestamp->setText("Timestamp: " + QString::number(_selectedDataHandle.getTimestamp()));
            _lblName->setText("Name: " + _selectedDataHandleName);
            _lblLocalMemoryFootprint->setText("Local Memory Footprint: " + humanizeBytes(_selectedDataHandle.getData()->getLocalMemoryFootprint()));
            _lblVideoMemoryFootprint->setText("Video Memory Footprint: " + humanizeBytes(_selectedDataHandle.getData()->getVideoMemoryFootprint()));
        }
        else {
            _lblTimestamp->setText("Timestamp: ");
            _lblName->setText("Name: ");
            _lblLocalMemoryFootprint->setText("Local Memory Footprint: ");
            _lblVideoMemoryFootprint->setText("Video Memory Footprint: ");
        }
    }

    QString DataContainerInspectorWidget::humanizeBytes(size_t numBytes) const {
        QString units[5] = { tr(" Bytes"), tr(" KB"), tr(" MB"), tr(" GB"), tr(" TB") };
        size_t index = 0;
        size_t remainder = 0;

        while (numBytes > 1024 && index < 5) {
            remainder = numBytes % 1024;
            numBytes /= 1024;
            ++index;
        }

        if (remainder != 0)
            return QString::number(numBytes) + "." + QString::number(remainder) + units[index];
        else
            return QString::number(numBytes) + units[index];
    }

    void DataContainerInspectorWidget::init() {
        if (_canvas != 0)
            _canvas->init();
    }

    void DataContainerInspectorWidget::deinit() {
        if (_canvas != 0)
            _canvas->deinit();
    }

}