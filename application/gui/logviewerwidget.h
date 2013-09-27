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

#ifndef LOGVIEWERWIDGET_H__
#define LOGVIEWERWIDGET_H__

#include "sigslot/sigslot.h"
#include "tgt/painter.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tbb/mutex.h"

#include "application/tools/bufferinglog.h"
#include "application/gui/loghighlighter.h"

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>

namespace campvis {

    class LogViewerWidget : public QWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerInspectorWidget.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit LogViewerWidget(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~LogViewerWidget();

        /**
         * Initializes the underlying log and registers it with tgt.
         */
        void init();

        /**
         * Deinitializes the underlying log and deregisters it from tgt.
         */
        void deinit();

    protected:
        /**
         * Setup the the log viewer's GUI
         */
        void setupGUI();

    public slots:
        /**
         * Append the given message to the log viewer
         *
         * This function adds the specified message to the log viewer's message cache and
         * displays it.
         *
         * \param message message to append to the log viewer
         */
        void appendMessage(const QString& message);

    private slots:
        /**
         * Display the given message in the log viewer
         *
         * \param message message to display in the log viewer
         */
        void displayMessage(const QString& message);

        /**
         * Delete all messages from the log viewer
         */
        void clearMessages();

        /**
         * Filter out log messages that don't contain the given text.
         *
         * \param text text to look for in log messages
         */
        void filterLogMessages(const QString& text);

    private:
        QVBoxLayout* _mainLayout;                       ///< Main layout of this widget
        QHBoxLayout* _controls_layout;                  ///< Layout storing this widget's controls
        QPushButton* _clear_button;                     ///< Button used for clearing the log display
        QLineEdit* _filter_line_edit;                   ///< Text field where filter terms are entered
        QLabel* _filter_label;                          ///< Button used for clearing the log display
        BufferingLog* _log;                             ///< Log buffering messages
        std::deque<QString> _logMessages;               ///< Queue storing a limited number of recent log messages
        QTextEdit* _logDisplay;                         ///< Widget displaying log messages
        const QRegExp* _filterRegExp;                   ///< Current filter regexp
        LogHighlighter* _logHighlighter;                ///< Highlighter for log messages
    };
}

#endif // LOGVIEWERWIDGET_H__
