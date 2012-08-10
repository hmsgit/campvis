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
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractprocessor.h"

namespace TUMVis {

    MainWindow::MainWindow(TumVisApplication* application)
        : QMainWindow()
        , _application(application)
        , _pipelineWidget(0)
    {
        tgtAssert(_application != 0, "Application must not be 0.");
        setup();
    }

    MainWindow::~MainWindow() {
        _application->s_PipelinesChanged.disconnect(this);
    }

    void MainWindow::setup() {
        _centralWidget = new QWidget(this);
        QHBoxLayout* boxLayout = new QHBoxLayout();
        boxLayout->setSpacing(4);

        _pipelineWidget = new PipelineTreeWidget(_centralWidget);
        boxLayout->addWidget(_pipelineWidget);

        _propCollectionWidget = new PropertyCollectionWidget(_centralWidget);
        boxLayout->addWidget(_propCollectionWidget);

        _centralWidget->setLayout(boxLayout);
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
        }
        else {
            emit updatePropCollectionWidget(0);
        }
    }

    QSize MainWindow::sizeHint() const {
        return QSize(800, 450);
    }

}