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

#include "loghighlighter.h"

namespace campvis {

    LogHighlighter::LogHighlighter(QTextEdit* parent)
        : QSyntaxHighlighter(parent)
        , _filterRegExp(0)
        , _logLevelRegExp(0)
        , _filterMatchFormat()
        , _rules(3)
        , _logLevelRules(5)
    {
        QBrush fgBrush = QBrush(Qt::white);
        // Firefox's green highlight
        QBrush bgBrush = QBrush(QColor(56, 216, 120));

        _filterMatchFormat.setBackground(bgBrush);
        _filterMatchFormat.setForeground(fgBrush);

        // Rules for highlighting date stamps, time stamps, and categories
        std::list< std::pair<QRegExp, QTextCharFormat> >::iterator it = _rules.begin();

        std::pair<QRegExp, QTextCharFormat>& dateStampRule = *it++;
        dateStampRule.first.setPattern("\\[(\\d{2}\\.){2}\\d{4}\\]");
        dateStampRule.second.setForeground(Qt::gray);

        std::pair<QRegExp, QTextCharFormat>& timeStampRule = *it++;
        timeStampRule.first.setPattern("\\[(\\d{2}:){2}\\d{2}\\]");
        timeStampRule.second.setForeground(Qt::gray);

        std::pair<QRegExp, QTextCharFormat>& categoryRule = *it++;
        categoryRule.first.setPattern("\\w+(\\.\\w+)*");
        categoryRule.second.setFontWeight(QFont::Bold);

        // Rules for highlighting log levels
        _logLevelRegExp = new QRegExp("\\((Debug|Info|Warning|Error|Fatal)\\)");
        std::list< std::pair<QString, QTextCharFormat> >::iterator logLevelIt = _logLevelRules.begin();

        std::pair<QString, QTextCharFormat>& debugRule = *logLevelIt++;
        debugRule.first = "Debug";
        debugRule.second.setForeground(Qt::blue);

        std::pair<QString, QTextCharFormat>& infoRule = *logLevelIt++;
        infoRule.first = "Info";
        infoRule.second.setForeground(Qt::darkGreen);

        std::pair<QString, QTextCharFormat>& warningRule = *logLevelIt++;
        warningRule.first = "Warning";
        warningRule.second.setForeground(Qt::darkYellow);

        std::pair<QString, QTextCharFormat>& errorRule = *logLevelIt++;
        errorRule.first = "Error";
        errorRule.second.setForeground(Qt::red);

        std::pair<QString, QTextCharFormat>& fatalRule = *logLevelIt++;
        fatalRule.first = "Fatal";
        fatalRule.second.setForeground(Qt::magenta);
    }

    LogHighlighter::~LogHighlighter() {
        setFilterRegExp(0);
    }

    void LogHighlighter::setFilterRegExp(const QRegExp* filterRegExp) {
        if (_filterRegExp != 0) {
            delete _filterRegExp;
        }

        _filterRegExp = filterRegExp;
    }

    void LogHighlighter::highlightBlock(const QString& text)
    {
        int offset = 0;
        std::list< std::pair<QRegExp, QTextCharFormat> >::iterator it = _rules.begin();

        do {
            std::pair<QRegExp, QTextCharFormat>& rule = *it++;
            offset = highlightRegExp(text, offset, rule.first, rule.second);

            if (offset == -1) {
                break;
            }
        } while (it != _rules.end());

        if (offset != -1) {
            highlightLogLevel(text, offset);
        }

        if (_filterRegExp != 0) {
            highlightFilterMatches(text);
        }
    }

    void LogHighlighter::highlightFilterMatches(const QString &text) {
        int pos = 0;

        while ((pos = _filterRegExp->indexIn(text, pos)) != -1) {
            int matchedLength = _filterRegExp->matchedLength();

            setFormat(pos, matchedLength, _filterMatchFormat);
            pos += matchedLength;
        }
    }

    int LogHighlighter::highlightRegExp(const QString &text, int offset, const QRegExp& regExp,
                                        const QTextCharFormat& format) {
        int pos = regExp.indexIn(text, offset);

        if (pos != -1) {
            int matchedLength = regExp.matchedLength();
            setFormat(pos, matchedLength, format);
            return pos + matchedLength;
        }

        return -1;
    }

    void LogHighlighter::highlightLogLevel(const QString &text, int offset) {
        int pos = _logLevelRegExp->indexIn(text, offset);

        if (pos != -1) {
            const QString& logLevel = _logLevelRegExp->cap(1);
            std::list< std::pair<QString, QTextCharFormat> >::iterator it = _logLevelRules.begin();

            do {
                std::pair<QString, QTextCharFormat>& rule = *it++;

                if (rule.first == logLevel) {
                    int matchedLength = _logLevelRegExp->matchedLength();
                    setFormat(pos, matchedLength, rule.second);
                    break;
                }
            } while (it != _logLevelRules.end());
        }
    }

}
