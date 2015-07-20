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

#ifndef LOGHIGHLIGHTER_H__
#define LOGHIGHLIGHTER_H__

#include <QTextEdit>
#include <QSyntaxHighlighter>

namespace campvis {

    class LogHighlighter : public QSyntaxHighlighter {

        Q_OBJECT

    public:

        /**
         * Constructs a LogHighlighter and installs it on parent.
         *
         * The specified QTextEdit also becomes the owner of the LogHighlighter.
         *
         * \param parent the QTextEdit to install the LogHighlighter on
         */
        explicit LogHighlighter(QTextEdit* parent);

        /**
         * Destructor.
         */
        ~LogHighlighter();

        /**
         * Highlights the given log block.
         *
         * This function is called when necessary by the rich text engine, i.e. on new/changed log
         * messages.
         *
         * \param text the log block to highlight
         */
        void highlightBlock(const QString &text);

        /**
         * Set the current filter regexp to highlight.
         *
         * LogHighlighter takes ownership of the given regexp.
         *
         * \param filterRegExp the current filter regexp
         */
        void setFilterRegExp(const QRegExp* filterRegExp);

    private:
        /**
         * Highlight all matches of the current regexp in the given log block.
         *
         * \param text the log block to highlight
         */
        void highlightFilterMatches(const QString& text);

        /**
         * Highlight log level information in the given log block.
         *
         * \param text the log block to highlight
         * \param offset the offset at which log level matching should start
         */
        void highlightLogLevel(const QString& text, int offset);

        /**
         * Highlight text matching the provided regexp in the given log block.
         *
         * \param text the log block to highlight
         * \param offset the offset at which matching should start
         * \param regExp the regular expression to match
         * \param format the format used to highlight matches
         * \return end of the matched string (the new offset), or -1 if nothing was matched
         */
        int highlightRegExp(const QString& text, int offset, const QRegExp& regExp, const QTextCharFormat& format);

        const QRegExp* _filterRegExp;                                     ///< Current filter regexp
        const QRegExp* _logLevelRegExp;                                   ///< Regexp matching log level strings
        QTextCharFormat _filterMatchFormat;                               ///< Format for highlighting filter matches
        std::list< std::pair<QRegExp, QTextCharFormat> > _rules;          ///< Set of general highlighting rules
        std::list< std::pair<QString, QTextCharFormat> > _logLevelRules;  ///< Highlighting rules for log levels
    };
}

#endif // LOGHIGHLIGHTER_H__
