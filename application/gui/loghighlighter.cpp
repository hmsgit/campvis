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
        delete _filterRegExp;
        delete _logLevelRegExp;
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
