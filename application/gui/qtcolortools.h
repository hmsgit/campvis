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

#ifndef QTCOLORTOOLS_H__
#define QTCOLORTOOLS_H__

#include "tgt/vector.h"
#include <QColor>
#include <QLabel>

namespace campvis {
    /**
     * Some helper methods for Qt colors.
     */
    struct QtColorTools {
        /**
         * Converts the QColor \a color to a tgt::col4.
         * \param   color   Color to convert
         * \return  tgt::col4 version of \a color.
         */
        static tgt::col4 toTgtColor(const QColor& color);

        /**
         * Converts the tgt::col4 color \a color to a QColor.
         * \param   color   Color to convert
         * \return  QColor version of \a color.
         */
        static QColor toQColor(const tgt::col4& color);
    };

    /**
     * Qt widget for showing and editing a QColor.
     * When clicking the widget a QColorDialog appears to change the color.
     */
    class ColorPickerWidget : public QLabel {
        Q_OBJECT

    public:
        /**
         * Creates a new ColorPickerWidget.
         * \param   color   Initial color
         * \param   parent  Parent Qt widget
         * \param   f       Qt WindowFlags
         */
        explicit ColorPickerWidget(const QColor& color = QColor::fromRgba(qRgba(0, 0, 0, 255)), QWidget* parent=0, Qt::WindowFlags f=0);

        /**
         * Returns the current color of the widget.
         * \return  _color
         */
        const QColor& color() const;
        
        /**
         * Sets the current color of the widget.
         * \param   color   New color of the widget.
         */
        void setColor(const QColor& color);

    signals:
        /**
         * Signal emitted when the color of the widget has changed.
         * \param   color   New color of the widget.
         */
        void colorChanged(const QColor& color);

    private:
        /// \override
        void mousePressEvent(QMouseEvent* e);
        /// \override
        void paintEvent(QPaintEvent* event);

        QColor _color;      ///< Current color of the widget
    };


}

#endif // QTCOLORTOOLS_H__
