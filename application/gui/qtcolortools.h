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
