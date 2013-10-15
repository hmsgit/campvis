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

#ifndef CAMPVISMAINWINDOW_H__
#define CAMPVISMAINWINDOW_H__

#include "sigslot/sigslot.h"
#include "application/campvisapplication.h"
#include "application/gui/pipelinetreewidget.h"
#include "application/gui/properties/propertycollectionwidget.h"
#include "application/gui/logviewerwidget.h"
#include "application/tools/qtexteditlog.h"
#include "application/ui_mainwindow.h"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QMdiArea>
#include <QScrollArea>
#include <vector>

namespace campvis {
    class DataContainerInspectorWidget;
    class DataContainerInspectorCanvas;

    /**
     * Main Window for the CAMPVis application.
     * Wraps a nice Qt GUI around the CampVisApplication instance given during creation.
     */
    class MainWindow : public QMainWindow, public sigslot::has_slots<> {
        Q_OBJECT

    public:
        /**
         * Creates a new TumVis Main window for \a application.
         * \param   application     CampVisApplication to create a GUI for.
         */
        explicit MainWindow(CampVisApplication* application);

        /**
         * Destructor, make sure to call before destroying the application
         */
        ~MainWindow();

        
        /**
         * Initializes all OpenGL related stuff
         */
        void init();

        /**
         * Deinitializes all OpenGL related stuff
         */
        void deinit();

        /**
         * Size hint for the default window size
         * \return QSize(800, 450)
         */
        QSize sizeHint() const;

        /**
         * Adds a widget of a visualization pipeline to the main window.
         * \param   name       the name of the visualization pipeline
         * \param   canvas     the pipeline's canvas
         */
        void addVisualizationPipelineWidget(const std::string& name, QWidget* canvas);

    protected:
        /**
         * Listens to resize events on _pipelinePropertiesWidget and resizes _pipelinePropertiesScrollArea accordingly
         * \param   watched    the object that caused the event
         * \param   event      the event to be filtered
         */
        bool eventFilter(QObject* watched, QEvent* event);

    signals:
        /// Qt signal for updating the PipelineWidget.
        void updatePipelineWidget(const std::vector<DataContainer*>&, const std::vector<AbstractPipeline*>&);

        /// Qt signal for updating the PropertyCollectionWidget
        void updatePropCollectionWidget(HasPropertyCollection*, DataContainer*);

    private slots:
        /**
         * Slot to be called by the PipelineWidget when the selected item changes.
         * \param   index   Index of the selected item
         */
        void onPipelineWidgetItemClicked(const QModelIndex& index);

        /**
         * Slot to be called when _btnExecute was clicked.
         */
        void onBtnExecuteClicked();

        /**
         * Slot to be called when _btnShowDataContainerInspector was clicked.
         */
        void onBtnShowDataContainerInspectorClicked();

        /// Slot to be called when _btnPipelineFactory was clicked;
        void onBtnPipelineFactoryClicked();

    private:

        /**
         * Slot to be called by the application when its collection of pipelines has changed.
         */
        void onPipelinesChanged();

        /**
         * Slot to be called by the application when its collection of DataContainers has changed.
         */
        void onDataContainersChanged();


        /**
         * Setup Qt GUI stuff
         */
        void setup();

        /**
         * Adds a widget to the top docking area of the main window.
         * This method creates a new dock with the specified name,
         * sets its widget to the given widget, and docks it in top
         * docking area of the main window. If there are already
         * other docks there, they're tabified with the new dock.
         * \param   name       the name that will be given to the created dock
         * \param   widget     the widget to add to the top docking area of the main window
         * \return  the dock created to store the provided widget
         */
        QDockWidget* dockPrimaryWidget(const std::string& name, QWidget* widget);

        Ui::MainWindow ui;                                  ///< Interface definition of the MainWindow

        CampVisApplication* _application;                    ///< Pointer to the application hosting the whole stuff

        QMdiArea* _mdiArea;                                 ///< MDI area (the window's central widget)
        QWidget* _containerWidget;                          ///< Widget to manage the app's DataContainers and pipelines
        QComboBox* _cbPipelineFactory;                      ///< Combobox for selecting the Pipelines from the PipelineFactory
        QPushButton* _btnPipelineFactory;                   ///< Button to add a Pipeline from the factory
        PipelineTreeWidget* _pipelineWidget;                ///< Widget for browsing the active pipelines
        QWidget* _pipelinePropertiesWidget;                 ///< Widget showing the selected pipeline's properties
        QScrollArea* _pipelinePropertiesScrollArea;         ///< Scroll area for _pipelinePropertiesWidget
        PropertyCollectionWidget* _propCollectionWidget;    ///< Widget for brosing the PropertyCollection of the selected pipeline/processor
        DataContainerInspectorWidget* _dcInspectorWidget;   ///< Widget for inspecting the DataContainer of the selected pipeline.
        QDockWidget* _dcInspectorDock;                      ///< Dock storing the above DataContainerInspectorWidget instance.

        QPushButton* _btnExecute;                           ///< Button to execute the selected pipeline/processor
        QPushButton* _btnShowDataContainerInspector;        ///< Button to show the DataContainerInspector for the selected pipeline

        AbstractPipeline* _selectedPipeline;                ///< currently selected pipeline
        AbstractProcessor* _selectedProcessor;              ///< currently selected processor
        DataContainer* _selectedDataContainer;              ///< currently selected DataContainer

        LogViewerWidget* _logViewer;                        ///< Widget displaying log messages
        std::vector<QDockWidget*> _primaryDocks;            ///< Docks located in top docking area of the main window
    };
}

#endif // CAMPVISMAINWINDOW_H__
