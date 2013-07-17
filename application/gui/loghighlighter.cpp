// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#include <QPalette>

namespace campvis {

    LogHighlighter::LogHighlighter(QTextEdit* parent)
        : QSyntaxHighlighter(parent)
        , _filterRegExp(0)
    {
        QBrush fgBrush = QBrush(Qt::white);
        // Firefox's green highlight
        QBrush bgBrush = QBrush(QColor(56, 216, 120));

        _filterMatchFormat.setBackground(bgBrush);
        _filterMatchFormat.setForeground(fgBrush);
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

}