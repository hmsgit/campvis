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

#include "mainwindow.h"

#include "cgt/assert.h"
#include "cgt/opengljobprocessor.h"

#include "application/campvisapplication.h"
#include "application/gui/datacontainerinspectorwidget.h"
#include "application/gui/datacontainerinspectorcanvas.h"
#include "application/gui/mdi/mdidockablewindow.h"
#include "application/gui/qtdatahandle.h"
#include "core/datastructures/datacontainer.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"
#include "core/pipeline/pipelinefactory.h"
#include "core/pipeline/processorfactory.h"
#include "core/tools/stringutils.h"

#include <QScrollBar>
#include <QFileDialog>

#include <fstream>

#include "scripting/luagen/properties/propertycollectionluascriptgenerator.h"
#include "scripting/luagen/properties/abstractpropertylua.h"


namespace campvis {

    MainWindow::MainWindow(CampVisApplication* application)
        : QMainWindow()
        , _application(application)
        , _mdiArea(0)
        , _containerWidget(0)
        , _cbPipelineFactory(0)
        , _btnPipelineFactory(0)
        , _pipelineWidget(0)
        , _propCollectionWidget(0)
        , _dcInspectorWidget(0)
        , _dcInspectorWindow(0)
        , _btnExecute(0)
        , _btnShowDataContainerInspector(0)
        , _selectedPipeline(0)
        , _selectedProcessor(0)
        , _selectedDataContainer(0)
        , _logViewer(0)
        , _scriptingConsoleWidget(nullptr)
        , _workflowWidget(nullptr)
    {
        cgtAssert(_application != 0, "Application must not be 0.");
        ui.setupUi(this);
        setup();
    }

    MainWindow::~MainWindow() {
        _application->s_PipelinesChanged.disconnect(this);
        _application->s_DataContainersChanged.disconnect(this);
        delete _dcInspectorWidget;
    }

    void MainWindow::setup() {
        qRegisterMetaType<QtDataHandle>("QtDataHandle");

        setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
        setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
        setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
        setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

        setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);

        _mdiArea = new MdiDockArea();
        _mdiArea->tileSubWindows();
        setCentralWidget(_mdiArea);

        _containerWidget = new QWidget(this);
        QGridLayout* _cwLayout = new QGridLayout(_containerWidget);

        int rowPosition = 0;
        _cbPipelineFactory = new QComboBox(_containerWidget);
        std::vector<std::string> registeredPipelines = PipelineFactory::getRef().getRegisteredPipelines();
        for (std::vector<std::string>::const_iterator it = registeredPipelines.begin(); it != registeredPipelines.end(); ++it)
            _cbPipelineFactory->addItem(QString::fromStdString(*it));
        _cwLayout->addWidget(_cbPipelineFactory, rowPosition, 0);

        _btnPipelineFactory = new QPushButton("Add Pipeline", _containerWidget);
        _cwLayout->addWidget(_btnPipelineFactory, rowPosition++, 1);

        _pipelineWidget = new PipelineTreeWidget(this);
        _containerWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        _cwLayout->addWidget(_pipelineWidget, rowPosition++, 0, 1, 2);

        _btnExecute = new QPushButton("Execute Selected Pipeline/Processor", _containerWidget);
        _cwLayout->addWidget(_btnExecute, rowPosition++, 0, 1, 2);

        _cbProcessorFactory = new QComboBox(_containerWidget);
        std::vector<std::string> registeredProcessors = ProcessorFactory::getRef().getRegisteredProcessors();
        for (std::vector<std::string>::const_iterator it = registeredProcessors.begin(); it != registeredProcessors.end(); ++it)
            _cbProcessorFactory->addItem(QString::fromStdString(*it));
        _cwLayout->addWidget(_cbProcessorFactory, rowPosition, 0);

        _btnProcessorFactory = new QPushButton("Add Processor", _containerWidget);
        _cwLayout->addWidget(_btnProcessorFactory, rowPosition++, 1);

        _btnShowDataContainerInspector = new QPushButton("Inspect DataContainer of Selected Pipeline", _containerWidget);
        _cwLayout->addWidget(_btnShowDataContainerInspector, rowPosition++, 0, 1, 2);

#ifdef CAMPVIS_HAS_SCRIPTING
        _btnLuaLoad = new QPushButton("Load Script", _containerWidget);
        _cwLayout->addWidget(_btnLuaLoad, rowPosition++, 0, 1, 2);
        _btnLuaSave = new QPushButton("Save Script", _containerWidget);
        _cwLayout->addWidget(_btnLuaSave, rowPosition++, 0, 1, 2);
        connect(
            _btnLuaLoad, SIGNAL(clicked()), 
            this, SLOT(onBtnLuaLoadClicked()));
        connect(
            _btnLuaSave, SIGNAL(clicked()), 
            this, SLOT(onBtnLuaSaveClicked()));
#else
#endif

        _containerWidget->setLayout(_cwLayout);
        ui.pipelineTreeDock->setWidget(_containerWidget);

        _pipelinePropertiesScrollArea = new QScrollArea(this);
        _pipelinePropertiesScrollArea->setWidgetResizable(true);
        _pipelinePropertiesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        _pipelinePropertiesScrollArea->setFrameStyle(QScrollArea::NoFrame);

        _pipelinePropertiesWidget = new QWidget(_pipelinePropertiesScrollArea);
        _pipelinePropertiesWidget->installEventFilter(this);
        _pipelinePropertiesScrollArea->setWidget(_pipelinePropertiesWidget);
        ui.pipelinePropertiesDock->setWidget(_pipelinePropertiesScrollArea);

        QVBoxLayout* rightLayout = new QVBoxLayout(_pipelinePropertiesWidget);
        rightLayout->setSpacing(4);
        _pipelinePropertiesWidget->setLayout(rightLayout);

        _propCollectionWidget = new PropertyCollectionWidget(this);
        rightLayout->addWidget(_propCollectionWidget);
        rightLayout->addStretch();

        _logViewer = new LogViewerWidget(this);
        ui.logViewerDock->setWidget(_logViewer);

#ifdef CAMPVIS_HAS_SCRIPTING
        _scriptingConsoleWidget = new ScriptingWidget(this);
        ui.scriptingConsoleDock->setWidget(_scriptingConsoleWidget);
        connect(_scriptingConsoleWidget, SIGNAL(s_commandExecuted(const QString&)), this, SLOT(onLuaCommandExecuted(const QString&)));
#else
        ui.scriptingConsoleDock->setVisible(false);
#endif

        _workflowWidget = new WorkflowControllerWidget(_application, this);
        ui.workflowDock->setWidget(_workflowWidget);
        ui.workflowDock->setVisible(! _application->_workflows.empty());

        _dcInspectorWidget = new DataContainerInspectorWidget();
        this->populateMainMenu();

        qRegisterMetaType< std::vector<DataContainer*> >("std::vector<DataContainer*>");
        qRegisterMetaType< std::vector<AbstractPipeline*> >("std::vector<AbstractPipeline*>");
        qRegisterMetaType< std::map<AbstractProperty*, QWidget*>::iterator >("PropertyWidgetMapIterator");
        connect(
            this, SIGNAL(updatePipelineWidget(const std::vector<DataContainer*>&, const std::vector<AbstractPipeline*>&)), 
            _pipelineWidget, SLOT(update(const std::vector<DataContainer*>&, const std::vector<AbstractPipeline*>&)));
        connect(
            _pipelineWidget, SIGNAL(itemChanged(const QModelIndex&)), 
            this, SLOT(onPipelineWidgetItemChanged(const QModelIndex&)));
        connect(
            this, SIGNAL(updatePropCollectionWidget(HasPropertyCollection*, DataContainer*)),
            _propCollectionWidget, SLOT(updatePropCollection(HasPropertyCollection*, DataContainer*)));
        connect(
            _btnExecute, SIGNAL(clicked()), 
            this, SLOT(onBtnExecuteClicked()));
        connect(
            _btnShowDataContainerInspector, SIGNAL(clicked()), 
            this, SLOT(onBtnShowDataContainerInspectorClicked()));
        connect(
            _btnPipelineFactory, SIGNAL(clicked()), 
            this, SLOT(onBtnPipelineFactoryClicked()));
        connect(
            _btnProcessorFactory, SIGNAL(clicked()), 
            this, SLOT(onBtnProcessorFactoryClicked()));

        _application->s_PipelinesChanged.connect(this, &MainWindow::onPipelinesChanged);
        _application->s_DataContainersChanged.connect(this, &MainWindow::onDataContainersChanged);

    }

    void MainWindow::populateMainMenu() {
        // Populate the file menu
        QMenuBar* menuBar = this->menuBar();
        QMenu* fileMenu = menuBar->addMenu(tr("&File"));
        fileMenu->addAction(tr("&Rebuild all Shaders from File"), this, SLOT(onRebuildShadersClicked()), QKeySequence(Qt::CTRL + Qt::Key_F5));
        fileMenu->addAction(tr("&Quit"), qApp, SLOT(closeAllWindows()), QKeySequence(Qt::CTRL + Qt::Key_Q));

        // Populate the visualizations menu
        QMenu* visualizationsMenu = _mdiArea->menu();
        visualizationsMenu->setTitle(tr("&Visualizations"));
        menuBar->addMenu(visualizationsMenu);

        // Populate the tools menu
        QMenu* toolsMenu = menuBar->addMenu(tr("&Tools"));
        toolsMenu->addAction(ui.pipelineTreeDock->toggleViewAction());
        toolsMenu->addAction(ui.pipelinePropertiesDock->toggleViewAction());
        toolsMenu->addAction(ui.logViewerDock->toggleViewAction());
        toolsMenu->addAction(ui.workflowDock->toggleViewAction());
    }

    bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
        if (watched == _pipelinePropertiesWidget && event->type() == QEvent::Resize) {
            _pipelinePropertiesScrollArea->setMinimumWidth(_pipelinePropertiesWidget->minimumSizeHint().width() +
                                                           _pipelinePropertiesScrollArea->verticalScrollBar()->width());
        }

        return false;
    }

    void MainWindow::onPipelinesChanged() {
        emit updatePipelineWidget(_application->_dataContainers, _application->_pipelines);
    }

    void MainWindow::onDataContainersChanged() {
        emit updatePipelineWidget(_application->_dataContainers, _application->_pipelines);
    }

    void MainWindow::onPipelineWidgetItemChanged(const QModelIndex& index) {
        if (index.isValid()) {
            // Yak, this is so ugly - another reason why GUI programming sucks...
            QVariant item = index.data(Qt::UserRole);
            if (item.isValid()) {
                HasPropertyCollection* ptr = static_cast<HasPropertyCollection*>(item.value<void*>());

                if (AbstractPipeline* pipeline = dynamic_cast<AbstractPipeline*>(ptr)) {
                    _selectedPipeline = pipeline;
                    _selectedProcessor = 0;
                    _selectedDataContainer = &pipeline->getDataContainer();
                }
                else if (AbstractProcessor* processor = dynamic_cast<AbstractProcessor*>(ptr)) {
                    _selectedProcessor = processor;

                    QVariant parentItem = index.parent().data(Qt::UserRole);
                    HasPropertyCollection* pptr = static_cast<HasPropertyCollection*>(parentItem.value<void*>());
                    if (AbstractPipeline* pipeline = dynamic_cast<AbstractPipeline*>(pptr)) {
                        _selectedPipeline = pipeline;
                        _selectedDataContainer = &pipeline->getDataContainer();
                    }
                }

                emit updatePropCollectionWidget(ptr, &_selectedPipeline->getDataContainer());

            }
            else {
                emit updatePropCollectionWidget(0, 0);
                _selectedDataContainer = 0;
            }

        }
        else {
            emit updatePropCollectionWidget(0, 0);
            _selectedDataContainer = 0;
        }
    }

    QSize MainWindow::sizeHint() const {
        return QSize(1000, 600);
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

    void MainWindow::onBtnLuaLoadClicked() {
#ifdef CAMPVIS_HAS_SCRIPTING
        const QString dialogCaption = QString::fromStdString("Select File");
        const QString directory = QString::fromStdString(".");
        const QString fileFilter = tr("All files (*)");

        QString filename = QFileDialog::getOpenFileName(QWidget::parentWidget(), dialogCaption, directory, fileFilter);
        if (filename != nullptr && _application->getLuaVmState() != nullptr) {
            _application->getLuaVmState()->execFile(filename.toStdString());
        }
#endif
    }

    void MainWindow::onBtnLuaSaveClicked() {
#ifdef CAMPVIS_HAS_SCRIPTING
        const QString dialogCaption = QString::fromStdString("Save File as");
        const QString directory = QString::fromStdString(".");
        const QString fileFilter = tr("All files (*)");

        QString filename = QFileDialog::getSaveFileName(QWidget::parentWidget(), dialogCaption, directory, fileFilter);

        if (filename != nullptr) {
            if (_selectedProcessor != 0 && _selectedPipeline != 0) {
                PropertyCollectionLuaScriptGenerator* _pcLua = new PropertyCollectionLuaScriptGenerator();
                
                std::string pipeScript = "pipeline = pipelines[\"" + _selectedPipeline->getName()+"\"]\n\n";
                for (size_t i = 0; i < _selectedPipeline->getProcessors().size(); i++) {
                    pipeScript += "proc = pipeline:getProcessor(" + StringUtils::toString(i) + ")\n";
                    AbstractProcessor* proc = _selectedPipeline->getProcessor(int(i));

                    _pcLua->updatePropCollection(proc, &_selectedPipeline->getDataContainer());
                    std::string res = _pcLua->getLuaScript(std::string(""), std::string("proc:"));
                    pipeScript += res;
                }
                if (pipeScript != "pipeline = pipelines[\"" + _selectedPipeline->getName()+"\"]\n\n") {
                    std::ofstream file;
                    file.open(filename.toStdString());
                    file << pipeScript.c_str();
                    file.close();
                }
                
                delete _pcLua;
            }
        }
#endif
    }

    void MainWindow::onBtnShowDataContainerInspectorClicked() {
        if (_selectedPipeline != 0) {
            if (_dcInspectorWindow == 0) {
                _dcInspectorWindow = _mdiArea->addWidget(_dcInspectorWidget);
                _dcInspectorWindow->setWindowTitle(tr("Data Container Inspector"));
            }

            _dcInspectorWidget->setDataContainer(&(_selectedPipeline->getDataContainer()));
            _dcInspectorWindow->show();
            _dcInspectorWindow->activateWindow();
        }
    }

    void MainWindow::init() {
        if (_dcInspectorWidget != 0)
            _dcInspectorWidget->init();

        _logViewer->init();

        if (_scriptingConsoleWidget)
            _scriptingConsoleWidget->init();
    }

    void MainWindow::deinit() {
        if (_dcInspectorWidget != 0)
            _dcInspectorWidget->deinit();

        _logViewer->deinit();

        if (_scriptingConsoleWidget)
            _scriptingConsoleWidget->deinit();
    }

    MdiDockableWindow * MainWindow::addVisualizationPipelineWidget(const std::string& name, QWidget* canvas) {
        MdiDockableWindow* dockableWindow = _mdiArea->addWidget(canvas);
        QString windowTitle = QString::fromStdString(name);
        dockableWindow->setWindowTitle(windowTitle);
        dockableWindow->show();
        return dockableWindow;
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

    void MainWindow::onBtnPipelineFactoryClicked() {
        std::string name = this->_cbPipelineFactory->currentText().toStdString();
        DataContainer* dc = _selectedDataContainer;
        if (dc == 0) {
            dc = _application->createAndAddDataContainer("DataContainer #" + StringUtils::toString(_application->_dataContainers.size() + 1));
        }
        AbstractPipeline* p = PipelineFactory::getRef().createPipeline(name, dc);
        _application->addPipeline(name, p);
    }

    void MainWindow::onBtnProcessorFactoryClicked() {
        cgt::OpenGLJobProcessor::ScopedSynchronousGlJobExecution jobGuard;

        std::string name = this->_cbProcessorFactory->currentText().toStdString();
        if (_selectedPipeline == nullptr) 
            return;

        IVec2Property temp = _selectedPipeline->getCanvasSize();
        IVec2Property *viewPort = new IVec2Property(temp.getName(), temp.getTitle(), temp.getValue(), temp.getMinValue(), temp.getMaxValue(), temp.getStepValue());
        AbstractProcessor* p = ProcessorFactory::getRef().createProcessor(name, viewPort);
        if (p == nullptr)
            return;
        
        p->init();
        _selectedPipeline->addProcessor(p);

        emit updatePipelineWidget(_application->_dataContainers, _application->_pipelines);
    }

    void MainWindow::onRebuildShadersClicked() {
        _application->rebuildAllShadersFromFiles();
    }

    void MainWindow::onLuaCommandExecuted(const QString& cmd) {
#ifdef CAMPVIS_HAS_SCRIPTING
        if (_application->getLuaVmState() != nullptr) {
            cgt::OpenGLJobProcessor::ScopedSynchronousGlJobExecution jobGuard;
            _application->getLuaVmState()->execString(cmd.toStdString());
        }
#endif
    }

    void MainWindow::setWorkflow(AbstractWorkflow* w) {
        ui.workflowDock->setVisible(true);
        _workflowWidget->setWorkflow(w);
    }

    void MainWindow::enableKioskMode() {
        ui.logViewerDock->hide();
        ui.pipelineTreeDock->hide();
        ui.pipelinePropertiesDock->hide();
        ui.scriptingConsoleDock->hide();
        ui.workflowDock->show();

    }

}
