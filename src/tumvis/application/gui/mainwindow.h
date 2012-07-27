#ifndef TUMVISMAINWINDOW_H__
#define TUMVISMAINWINDOW_H__

#include "sigslot/sigslot.h"
#include "application/tumvisapplication.h"
#include "application/gui/pipelinetreewidget.h"
#include "application/gui/propertycollectionwidget.h"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>

namespace TUMVis {
    /**
     * Main Window for the TUMVis application.
     * Wraps a nice Qt GUI around the TumVisApplication instance given during creation.
     */
    class MainWindow : public QMainWindow, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new TumVis Main window for \a application.
         * \param   application     TumVisApplication to create a GUI for.
         */
        MainWindow(TumVisApplication* application);

        /**
         * Destructor, make sure to call before destroying the application
         */
        ~MainWindow();


        /**
         * Slot to be called by the application when its collection of pipelines has changed.
         */
        void onPipelinesChanged();

    signals:
        /// Qt signal for updating the PipelineWidget.
        void updatePipelineWidget(const std::vector<AbstractPipeline*>&);
        /// Qt signal for updating the PropertyCollectionWidget
        void updatePropCollectionWidget(PropertyCollection*);

    private slots:
        /**
         * Slot to be called by the PipelineWidget when the selected item changes.
         * \param   index   Index of the selected item
         */
        void onPipelineWidgetItemClicked(const QModelIndex& index);

    private:
        /**
         * Setup Qt GUI stuff
         */
        void setup();

        TumVisApplication* _application;                    ///< Pointer to the application hosting the whole stuff

        QWidget* _centralWidget;                            ///< Central widget of the MainWindow
        PipelineTreeWidget* _pipelineWidget;                ///< Widget for browsing the active pipelines
        PropertyCollectionWidget* _propCollectionWidget;    ///< Widget for brosing the PropertyCollection of the selected pipeline/processor
    };
}

#endif // TUMVISMAINWINDOW_H__
