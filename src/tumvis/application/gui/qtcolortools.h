#ifndef QTCOLORTOOLS_H__
#define QTCOLORTOOLS_H__

#include "tgt/vector.h"
#include <QColor>
#include <QLabel>

namespace TUMVis {
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
        ColorPickerWidget(const QColor& color = QColor::fromRgba(qRgba(0, 0, 0, 255)), QWidget* parent=0, Qt::WindowFlags f=0);

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
