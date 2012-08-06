#include "abstractpropertywidget.h"
#include "core/properties/abstractproperty.h"

namespace TUMVis {

    AbstractPropertyWidget::AbstractPropertyWidget(AbstractProperty* property, QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _property(property)
        , _ignorePropertyUpdates(false)
        , _layout(0)
    {
        _titleLabel = new QLabel(QString::fromStdString(_property->getTitle() + ":"), this);

        _layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
        _layout->setSpacing(16);
        _layout->addWidget(_titleLabel, 0);

        _property->s_changed.connect(this, &AbstractPropertyWidget::onPropertyChanged);
    }

    AbstractPropertyWidget::~AbstractPropertyWidget() {
        _property->s_changed.disconnect(this);
    }

    void AbstractPropertyWidget::addWidget(QWidget* widget) {
        _layout->addWidget(widget, 1);
    }

    void AbstractPropertyWidget::onPropertyChanged(const AbstractProperty* property) {
        if (!_ignorePropertyUpdates)
            updateWidgetFromProperty();
    }
}