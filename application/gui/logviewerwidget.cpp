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

#include "logviewerwidget.h"

namespace campvis {

    LogViewerWidget::LogViewerWidget(QWidget* parent)
        : QWidget(parent)
        , _filterRegExp(0)
    {
        setupGUI();

        _log = new BufferingLog(100, this);
        _log->addCat("", true);

        connect(_log, SIGNAL(s_messageAppended(const QString&)),
                this, SLOT(appendMessage(const QString&)));
    }

    LogViewerWidget::~LogViewerWidget() {
    }

    void LogViewerWidget::setupGUI() {
        setWindowTitle(tr("Log Viewer"));

        _mainLayout = new QVBoxLayout();
        setLayout(_mainLayout);

        _controls_layout = new QHBoxLayout();
        _mainLayout->addLayout(_controls_layout);

        _filter_label = new QLabel(tr("&Filter:"));
        _controls_layout->addWidget(_filter_label);

        _filter_line_edit = new QLineEdit();
        _filter_line_edit->setPlaceholderText(tr("Start typing here to filter log messages"));

        _filter_label->setBuddy(_filter_line_edit);
        _controls_layout->addWidget(_filter_line_edit);
        _controls_layout->addStretch();

        _clear_button = new QPushButton(tr("&Clear"));
        _controls_layout->addWidget(_clear_button);

        _logDisplay = new QTextEdit();
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
    }

    void LogViewerWidget::init() {
        LogMgr.addLog(_log);
    }

    void LogViewerWidget::deinit() {
        LogMgr.removeLog(_log);
    }

    void LogViewerWidget::appendMessage(const QString& message)
    {
        _logMessages.push_back(message);
        displayMessage(message);
    }

    void LogViewerWidget::displayMessage(const QString& message)
    {
        if (_filterRegExp == 0 || _filterRegExp->indexIn(message) != -1) {
            _logDisplay->append(message);
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

        for (std::deque<QString>::iterator it = _logMessages.begin(); it != _logMessages.end(); it++) {
            displayMessage(*it);
        }
    }

}
