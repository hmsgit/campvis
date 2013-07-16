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

#ifndef LOGHIGHLIGHTER_H__
#define LOGHIGHLIGHTER_H__

#include <QTextEdit>
#include <QSyntaxHighlighter>

namespace campvis {

    class LogHighlighter : public QSyntaxHighlighter {

        Q_OBJECT;

    public:

        /**
         * Constructs a LogHighlighter and installs it on parent.
         *
         * The specified QTextEdit also becomes the owner of the LogHighlighter.
         *
         * \param parent the QTextEdit to install the LogHighlighter on
         */
        LogHighlighter(QTextEdit* parent);

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
         * \param filterRegExp the current filter regexp
         */
        void setFilterRegExp(const QRegExp* filterRegExp);

    private:
        /**
         * Highlight all matches of the current regexp in the given log block.
         *
         * \param text the log block to highlight
         */
        void highlightFilterMatches(const QString &text);

        const QRegExp* _filterRegExp;                   ///< Current filter regexp
        QTextCharFormat _filterMatchFormat;             ///< Format for highlighting filter matches
    };
}

#endif // LOGHIGHLIGHTER_H__
