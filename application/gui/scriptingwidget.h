// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef SCRIPTINGWIDGET_H__
#define SCRIPTINGWIDGET_H__

#include "sigslot/sigslot.h"
#include "cgt/logmanager.h"
#include "cgt/painter.h"
#include "tbb/mutex.h"

#include "application/gui/completinglualineedit.h"
#include "application/tools/bufferinglog.h"
#include "application/gui/loghighlighter.h"

#include <deque>

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
    /**
     * Qt widget providing a console-like interface to the Lua VM of CampvisApplication.
     */
    class ScriptingWidget : public QWidget, cgt::Log {
        Q_OBJECT;

    public:
        /**
         * Creates a new ScriptingWidget.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit ScriptingWidget(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~ScriptingWidget();

        bool isOpen() { return true; }
        
        /**
         * Initializes the underlying log and registers it with cgt.
         */
        void init();

        /**
         * Deinitializes the underlying log and deregisters it from cgt.
         */
        void deinit();

        CompletingLuaLineEdit* _editCommand;            ///< Text field to enter Lua commands

    protected:
        /**
         * Setup the the log viewer's GUI
         */
        void setupGUI();

        bool eventFilter(QObject* obj, QEvent* event);

        void logFiltered(const std::string &cat, cgt::LogLevel level, const std::string& msg, const std::string& extendedInfo="");

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
         * Delete all messages from the log viewer
         */
        void clearLog();

        void execute();

    signals:
        void s_commandExecuted(const QString& cmd);

    private:
        QTextEdit* _consoleDisplay;         ///< Text edit to hold the console output
        QPushButton* _btnExecute;           ///< Button to execute command
        QPushButton* _btnClear;             ///< Button to clear the console output

        std::deque<QString> _history;       ///< History of executed commands
        int _currentPosition;               ///< Current position in command history
    };
}

#endif // SCRIPTINGWIDGET_H__
