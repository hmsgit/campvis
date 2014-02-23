// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "logviewerwidget.h"

namespace campvis {

    LogViewerWidget::LogViewerWidget(QWidget* parent)
        : QWidget(parent)
        , _filterRegExp(0)
    {
        setupGUI();

        _log = new BufferingLog(100, this);
        _log->addCat("", true, tgt::Debug);

        connect(_log, SIGNAL(s_messageAppended(const QString&, int)),
                this, SLOT(appendMessage(const QString&, int)));
    }

    LogViewerWidget::~LogViewerWidget() {
        delete _logHighlighter;
        delete _controls_layout;
    }

    void LogViewerWidget::setupGUI() {
        setWindowTitle(tr("Log Viewer"));

        _mainLayout = new QVBoxLayout(this);

        _controls_layout = new QHBoxLayout();
        _mainLayout->addLayout(_controls_layout);

        _filter_label = new QLabel(tr("&Filter:"), this);
        _controls_layout->addWidget(_filter_label);

        _filter_line_edit = new QLineEdit(this);
        _filter_line_edit->setPlaceholderText(tr("Start typing here to filter log messages"));

        _filter_label->setBuddy(_filter_line_edit);
        _controls_layout->addWidget(_filter_line_edit);

        _cbLogLevel = new QComboBox(this);
        _cbLogLevel->addItem("Debug",   static_cast<int>(tgt::Debug));
        _cbLogLevel->addItem("Info",    static_cast<int>(tgt::Info));
        _cbLogLevel->addItem("Warning", static_cast<int>(tgt::Warning));
        _cbLogLevel->addItem("Error",   static_cast<int>(tgt::Error));
        _cbLogLevel->addItem("Fatal",   static_cast<int>(tgt::Fatal));
        _cbLogLevel->setCurrentIndex(1);

        QLabel* lblLogLevel = new QLabel("Minimum Log Level:");
        lblLogLevel->setBuddy(_cbLogLevel);

        _controls_layout->addWidget(lblLogLevel);
        _controls_layout->addWidget(_cbLogLevel);

        _controls_layout->addStretch();

        _clear_button = new QPushButton(tr("&Clear"), this);
        _controls_layout->addWidget(_clear_button);

        _logDisplay = new QTextEdit(this);
        _logDisplay->setReadOnly(true);
        _mainLayout->addWidget(_logDisplay);

        // Use the system's default monospace font at the default size in the log viewer
        QFont monoFont = QFont("Monospace");
        monoFont.setStyleHint(QFont::TypeWriter);
        monoFont.setPointSize(QFont().pointSize() + 1);

        _logDisplay->document()->setDefaultFont(monoFont);
        _logHighlighter = new LogHighlighter(_logDisplay);

        connect(_clear_button, SIGNAL(clicked()), this, SLOT(clearMessages()));
        connect(_filter_line_edit, SIGNAL(textEdited(const QString&)), this, SLOT(filterLogMessages(const QString&)));
        connect(_cbLogLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(onLogLevelChanged(int)));
    }

    void LogViewerWidget::init() {
        LogMgr.addLog(_log);
    }

    void LogViewerWidget::deinit() {
        LogMgr.removeLog(_log);
    }

    void LogViewerWidget::appendMessage(const QString& message, int level)
    {
        LogEntry entry = { message, level };
        _logMessages.push_back(entry);
        displayMessage(entry);
    }

    void LogViewerWidget::displayMessage(const LogViewerWidget::LogEntry& message)
    {
        int filterLevel = _cbLogLevel->itemData(_cbLogLevel->currentIndex()).toInt();
        
        if ((message._level >= filterLevel) && (_filterRegExp == 0 || _filterRegExp->indexIn(message._message) != -1)) {
            _logDisplay->append(message._message);
        }
    }

    void LogViewerWidget::clearMessages()
    {
        _logDisplay->clear();
        _logMessages.clear();
    }

    void LogViewerWidget::filterLogMessages(const QString& text)
    {
        if (text.length() == 0) {
            _filterRegExp = 0;
        } else {
            _filterRegExp = new QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString);
        }

        _logDisplay->clear();
        _logHighlighter->setFilterRegExp(_filterRegExp);

        for (std::deque<LogEntry>::iterator it = _logMessages.begin(); it != _logMessages.end(); it++) {
            displayMessage(*it);
        }
    }

    void LogViewerWidget::onLogLevelChanged(int) {
        filterLogMessages(_filter_line_edit->text());
    }

}
