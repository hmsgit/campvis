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

#ifndef LOGVIEWERWIDGET_H__
#define LOGVIEWERWIDGET_H__

#include "sigslot/sigslot.h"
#include "cgt/painter.h"
#include "tbb/mutex.h"

#include "application/tools/bufferinglog.h"
#include "application/gui/loghighlighter.h"

#include <QComboBox>
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
        struct LogEntry {
            QString _message;
            int _level;
        };

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
         * Initializes the underlying log and registers it with cgt.
         */
        void init();

        /**
         * Deinitializes the underlying log and deregisters it from cgt.
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
        void appendMessage(const QString& message, int level);

    private slots:
        /**
         * Display the given message in the log viewer
         *
         * \param message message to display in the log viewer
         */
        void displayMessage(const LogEntry& message);

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

        /**
         * Slot when _cbLogLevel has changed
         * \param int   current index of _cbLogLevel
         */
        void onLogLevelChanged(int);

    private:
        QVBoxLayout* _mainLayout;                       ///< Main layout of this widget
        QHBoxLayout* _controls_layout;                  ///< Layout storing this widget's controls
        QPushButton* _clear_button;                     ///< Button used for clearing the log display
        QLineEdit* _filter_line_edit;                   ///< Text field where filter terms are entered
        QLabel* _filter_label;                          ///< Button used for clearing the log display
        BufferingLog* _log;                             ///< Log buffering messages
        std::deque<LogEntry> _logMessages;              ///< Queue storing a limited number of recent log messages
        QTextEdit* _logDisplay;                         ///< Widget displaying log messages
        const QRegExp* _filterRegExp;                   ///< Current filter regexp
        LogHighlighter* _logHighlighter;                ///< Highlighter for log messages

        QComboBox* _cbLogLevel;                         ///< Minimum Log Level for Log viewer messages
    };
}

#endif // LOGVIEWERWIDGET_H__
