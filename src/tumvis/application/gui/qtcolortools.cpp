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