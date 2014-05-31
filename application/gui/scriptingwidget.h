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

#ifndef SCRIPTINGWIDGET_H__
#define SCRIPTINGWIDGET_H__

#include "sigslot/sigslot.h"
#include "tgt/painter.h"
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

    class ScriptingWidget : public QWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new DataContainerInspectorWidget.
         * \param   parent          Parent Qt widget, may be 0 (default)
         */
        explicit ScriptingWidget(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~ScriptingWidget();

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

        /**
         * Delete all messages from the log viewer
         */
        void clearLog();

        void execute();

    signals:
        void s_commandExecuted(const QString& cmd);

    private:
        QTextEdit* _consoleDisplay;         ///< Text edit to hold the console output
        QLineEdit* _editCommand;            ///< Text field to enter Lua commands
        QPushButton* _btnExecute;           ///< Button to execute command
        QPushButton* _btnClear;             ///< Button to clear the console output
    };
}

#endif // SCRIPTINGWIDGET_H__
