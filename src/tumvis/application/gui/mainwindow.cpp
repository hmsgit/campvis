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

#include "mainwindow.h"

#include "tgt/assert.h"
#include "application/tumvisapplication.h"
#include "application/gui/datacontainerinspectorwidget.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"

namespace TUMVis {

    MainWindow::MainWindow(TumVisApplication* application)
        : QMainWindow()
        , _application(application)
        , _centralWidget(0)
        , _pipelineWidget(0)
        , _propCollectionWidget(0)
        , _dcInspectorWidget(0)
        , _btnExecute(0)
        , _btnShowDataContainerInspector(0)
        , _selectedPipeline(0)
        , _selectedProcessor(0)
    {
        tgtAssert(_application != 0, "Application must not be 0.");
        setup();
    }

    MainWindow::~MainWindow() {
        _application->s_PipelinesChanged.disconnect(this);
    }

    void MainWindow::setup() {
        _centralWidget = new QWidget(this);
        QHBoxLayout* mainLayout = new QHBoxLayout();
        mainLayout->setSpacing(4);

        _pipelineWidget = new PipelineTreeWidget(_centralWidget);
        mainLayout->addWidget(_pipelineWidget);

        QWidget* rightWidget = new QWidget(_centralWidget);
        mainLayout->addWidget(rightWidget);

        QVBoxLayout* rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(4);
        rightWidget->setLayout(rightLayout);

        _btnExecute = new QPushButton("Execute Selected Pipeline/Processor", rightWidget);
        rightLayout->addWidget(_btnExecute);

        _btnShowDataContainerInspector = new QPushButton("Inspect DataContainer of Selected Pipeline", rightWidget);
        rightLayout->addWidget(_btnShowDataContainerInspector);

        _propCollectionWidget = new PropertyCollectionWidget(_centralWidget);
        rightLayout->addWidget(_propCollectionWidget);

        _centralWidget->setLayout(mainLayout);
        setCentralWidget(_centralWidget);

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
                if (_dcInspectorWidget != 0)
                    onBtnShowDataContainerInspectorClicked();
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
        }
        else if (_selectedPipeline != 0) {
            
        }
    }

    void MainWindow::onBtnShowDataContainerInspectorClicked() {
        if (_selectedPipeline != 0) {
            if (_dcInspectorWidget == 0) {
                _dcInspectorWidget = new DataContainerInspectorWidget();
            }
            _dcInspectorWidget->setDataContainer(&(_selectedPipeline->getDataContainer()));
            _dcInspectorWidget->show();
        }
    }

}