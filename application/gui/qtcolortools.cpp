// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "qtcolortools.h"

#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>

namespace campvis {

    cgt::col4 QtColorTools::toTgtColor(const QColor& color) {
        return cgt::col4(color.red(),color.green(),color.blue(), color.alpha());
    }

    QColor QtColorTools::toQColor(const cgt::col4& color) {
        return QColor(color.r, color.g, color.b, color.a);
    }



    ColorPickerWidget::ColorPickerWidget(const QColor& color /*= QColor::fromRgba(qRgba(0, 0, 0, 255))*/, QWidget* parent/*=0*/, Qt::WindowFlags f/*=0*/)
        : QLabel(parent, f)
        , _color(color)
    {
        setMinimumHeight(16);
        setMinimumWidth(32);
    }

    const QColor& ColorPickerWidget::color() const {
        return _color;
    }

    void ColorPickerWidget::setColor(const QColor& color) {
        _color = color;
        update();
    }

    void ColorPickerWidget::mousePressEvent(QMouseEvent* e) {
        if (e->button() == Qt::LeftButton) {
            // launch a color picker dialog and set new color on success
            QColor newColor = QColorDialog::getColor(_color, this, "Select New Color", QColorDialog::ShowAlphaChannel);
            if(newColor.isValid()) {
                _color = newColor;
                emit colorChanged(_color);
            }
        }
    }

    void ColorPickerWidget::paintEvent(QPaintEvent* event) {
        QPainter painter(this);
        painter.setBrush(_color);
        painter.drawRect(1, 1, rect().width() - 2, rect().height() - 2);
    }

}