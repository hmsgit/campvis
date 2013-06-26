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

#include "mainwindow.h"

#include "tgt/assert.h"
#include "application/campvisapplication.h"
#include "application/gui/datacontainerinspectorwidget.h"
#include "application/gui/datacontainerinspectorcanvas.h"
#include "application/gui/qtdatahandle.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"

namespace campvis {

    MainWindow::MainWindow(CampVisApplication* application)
        : QMainWindow()
        , _application(application)
        , _centralWidget(0)
        , _pipelineWidget(0)
        , _propCollectionWidget(0)
        , _dcInspectorWidget(0)
        , _dcInspectorDock(0)
        , _btnExecute(0)
        , _btnShowDataContainerInspector(0)
        , _selectedPipeline(0)
        , _selectedProcessor(0)
        , _logViewer(0)
        , _log(0)
    {
        tgtAssert(_application != 0, "Application must not be 0.");
        ui.setupUi(this);
        setup();
    }

    MainWindow::~MainWindow() {
        _application->s_PipelinesChanged.disconnect(this);
        delete _dcInspectorWidget;
    }

    void MainWindow::setup() {
        qRegisterMetaType<QtDataHandle>("QtDataHandle");

        setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
        setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
        setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
        setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

        setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);

        _pipelineWidget = new PipelineTreeWidget();
        ui.pipelineTreeDock->setWidget(_pipelineWidget);

        QWidget* rightWidget = new QWidget();
        ui.pipelinePropertiesDock->setWidget(rightWidget);

        QVBoxLayout* rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(4);
        rightWidget->setLayout(rightLayout);

        _btnExecute = new QPushButton("Execute Selected Pipeline/Processor", rightWidget);
        rightLayout->addWidget(_btnExecute);

        _btnShowDataContainerInspector = new QPushButton("Inspect DataContainer of Selected Pipeline", rightWidget);
        rightLayout->addWidget(_btnShowDataContainerInspector);

        _propCollectionWidget = new PropertyCollectionWidget(_centralWidget);
        rightLayout->addWidget(_propCollectionWidget);

        _logViewer = new QTextEdit();
        _logViewer->setReadOnly(true);
        ui.logViewerDock->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        ui.logViewerDock->setWidget(_logViewer);

        _dcInspectorWidget = new DataContainerInspectorWidget();

        connect(
            this, SIGNAL(updatePipelineWidget(const std::vector<AbstractPipeline*>&)), 
            _pipelineWidget, SLOT(update(const std::vector<AbstractPipeline*>&)));
        connect(
            _pipelineWidget, SIGNAL(clicked(const QModelIndex&)), 
            this, SLOT(onPipelineWidgetItemClicked(const QModelIndex&)));
        connect(
            this, SIGNAL(updatePropCollectionWidget(HasPropertyCollection*)),
            _propCollectionWidget, SLOT(updatePropCollection(HasPropertyCollection*)));
        connect(
            _btnExecute, SIGNAL(clicked()), 
            this, SLOT(onBtnExecuteClicked()));
        connect(
            _btnShowDataContainerInspector, SIGNAL(clicked()), 
            this, SLOT(onBtnShowDataContainerInspectorClicked()));
        _application->s_PipelinesChanged.connect(this, &MainWindow::onPipelinesChanged);
    }

    void MainWindow::onPipelinesChanged() {
        emit updatePipelineWidget(_application->_pipelines);
    }

    void MainWindow::onPipelineWidgetItemClicked(const QModelIndex& index) {
        if (index.isValid()) {
            // Yak, this is so ugly - another reason why GUI programming sucks...
            QVariant item = index.data(Qt::UserRole);
            HasPropertyCollection* ptr = static_cast<HasPropertyCollection*>(item.value<void*>());
            emit updatePropCollectionWidget(ptr);

            if (AbstractPipeline* pipeline = dynamic_cast<AbstractPipeline*>(ptr)) {
            	_selectedPipeline = pipeline;
                _selectedProcessor = 0;
            }
            else if (AbstractProcessor* processor = dynamic_cast<AbstractProcessor*>(ptr)) {
                _selectedProcessor = processor;
            }
        }
        else {
            emit updatePropCollectionWidget(0);
        }
    }

    QSize MainWindow::sizeHint() const {
        return QSize(800, 450);
    }

    void MainWindow::onBtnExecuteClicked() {
        if (_selectedProcessor != 0 && _selectedPipeline != 0) {
            // this is not as trivial as it seems:
            // We need the pipeline, probably an OpenGL context...
            _selectedProcessor->invalidate(AbstractProcessor::INVALID_RESULT);
        }
        else if (_selectedPipeline != 0) {
            for (std::vector<AbstractProcessor*>::const_iterator it = _selectedPipeline->getProcessors().begin(); it != _selectedPipeline->getProcessors().end(); ++it) {
                (*it)->invalidate(AbstractProcessor::INVALID_RESULT);
            }
        }
    }

    void MainWindow::onBtnShowDataContainerInspectorClicked() {
        if (_selectedPipeline != 0) {
            if (_dcInspectorDock == 0) {
                _dcInspectorDock = dockPrimaryWidget("Data Container inspector", _dcInspectorWidget);
            } else {
                // Activate the dock's tab
                _dcInspectorDock->setVisible(true);
                _dcInspectorDock->raise();
            }

            _dcInspectorWidget->setDataContainer(&(_selectedPipeline->getDataContainer()));
        }
    }

    void MainWindow::init() {
        if (_dcInspectorWidget != 0)
            _dcInspectorWidget->init();

        _log = new QTextEditLog(_logViewer);
        _log->addCat("", true);
        LogMgr.addLog(_log);
    }

    void MainWindow::deinit() {
        if (_dcInspectorWidget != 0)
            _dcInspectorWidget->deinit();

        LogMgr.removeLog(_log);
    }

    void MainWindow::addVisualizationPipelineWidget(const std::string& name, QWidget* widget) {
        dockPrimaryWidget(name, widget);
    }

    QDockWidget* MainWindow::dockPrimaryWidget(const std::string& name, QWidget* widget) {
        QDockWidget* dockWidget = new QDockWidget(QString::fromStdString(name));
        dockWidget->setWidget(widget);
        dockWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        if (_primaryDocks.empty()) {
            addDockWidget(Qt::TopDockWidgetArea, dockWidget);
        } else {
            tabifyDockWidget(_primaryDocks.back(), dockWidget);
            // Activate the dock's tab
            dockWidget->setVisible(true);
            dockWidget->raise();
        }

        _primaryDocks.push_back(dockWidget);
        return dockWidget;
    }

}
