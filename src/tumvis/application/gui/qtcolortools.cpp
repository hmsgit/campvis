// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "qtcolortools.h"

#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>

namespace TUMVis {

    tgt::col4 QtColorTools::toTgtColor(const QColor& color) {
        return tgt::col4(color.red(),color.green(),color.blue(), color.alpha());
    }

    QColor QtColorTools::toQColor(const tgt::col4& color) {
        return QColor(color.r, color.g, color.b, color.a);
    }



    ColorPickerWidget::ColorPickerWidget(const QColor& color /*= QColor::fromRgba(qRgba(0, 0, 0, 255))*/, QWidget* parent/*=0*/, Qt::WindowFlags f/*=0*/)
        : QLabel(parent, f)
        , _color(color)
    {
        setMinimumHeight(16);
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