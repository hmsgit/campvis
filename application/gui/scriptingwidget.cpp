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

#include "scriptingwidget.h"

#include <QCompleter>
#include <QDirModel>
#include <QKeyEvent>

namespace campvis {

    ScriptingWidget::ScriptingWidget(QWidget* parent)
        : QWidget(parent)
        , _consoleDisplay(nullptr)
        , _editCommand(nullptr)
        , _btnExecute(nullptr)
        , _btnClear(nullptr)
        , _currentPosition(-1)
    {
        setupGUI();
        addCat("", true, cgt::LuaInfo);
    }

    ScriptingWidget::~ScriptingWidget() {
    }

    void ScriptingWidget::init() {
        LogMgr.addLog(this);
    }

    void ScriptingWidget::deinit() {
        LogMgr.removeLog(this);
    }

    void ScriptingWidget::setupGUI() {
        setWindowTitle(tr("Scripting Console"));

        QHBoxLayout* mainLayout = new QHBoxLayout(this);

        QVBoxLayout* leftColumnLayout = new QVBoxLayout();
        mainLayout->addLayout(leftColumnLayout);

        QHBoxLayout* controlsLayout = new QHBoxLayout();
        leftColumnLayout->addLayout(controlsLayout);

        _consoleDisplay = new QTextEdit(this);
        _consoleDisplay->setReadOnly(true);
        leftColumnLayout->addWidget(_consoleDisplay);

        // Use the system's default monospace font at the default size in the log viewer
        QFont monoFont = QFont("Monospace");
        monoFont.setStyleHint(QFont::TypeWriter);
        monoFont.setPointSize(QFont().pointSize() + 1);

        _consoleDisplay->document()->setDefaultFont(monoFont);
        _editCommand = new CompletingLuaLineEdit(nullptr, this);
        _editCommand->setPlaceholderText(tr("Enter Lua commands here..."));
        _editCommand->installEventFilter(this);
        controlsLayout->addWidget(_editCommand);

        _btnExecute = new QPushButton(tr("&Execute"), this);
        controlsLayout->addWidget(_btnExecute);

        _btnClear = new QPushButton(tr("&Clear"), this);
        controlsLayout->addWidget(_btnClear);


        connect(_btnClear, SIGNAL(clicked()), this, SLOT(clearLog()));
        connect(_btnExecute, SIGNAL(clicked()), this, SLOT(execute()));
        connect(_editCommand, SIGNAL(returnPressed()), this, SLOT(execute()));
    }

    void ScriptingWidget::appendMessage(const QString& message) {
        _consoleDisplay->append(message);
    }

    void ScriptingWidget::clearLog() {
        _consoleDisplay->clear();
    }

    void ScriptingWidget::execute() {
        QString command = _editCommand->text();
        appendMessage(tr("> ") + command);

        emit s_commandExecuted(command);

        _history.push_front(command);
        _currentPosition = -1;
        _editCommand->clear();
    }

    bool ScriptingWidget::eventFilter(QObject* obj, QEvent* event) {
        if (obj == _editCommand && event->type() == QEvent::KeyPress) {
            QKeyEvent* e = static_cast<QKeyEvent*>(event);
            if (e->key() == Qt::Key_Up && _currentPosition < static_cast<int>(_history.size())-1) {
                ++_currentPosition;
                _editCommand->setText(_history[_currentPosition]);
                return true;
            }
            if (e->key() == Qt::Key_Down && _currentPosition >= 0) {
                --_currentPosition;
                if (_currentPosition >= 0)
                    _editCommand->setText(_history[_currentPosition]);
                else
                    _editCommand->setText(tr(""));

                return true;
            }
        }

        return QObject::eventFilter(obj, event);
    }

    void ScriptingWidget::logFiltered(const std::string &cat, cgt::LogLevel level, const std::string& msg, const std::string& extendedInfo/*=""*/) {
        if (level == cgt::LuaInfo || level == cgt::LuaError) {
            appendMessage(QString::fromStdString(msg));
        }
    }




}
