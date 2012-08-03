#include "stringpropertywidget.h"

namespace TUMVis {
    StringPropertyWidget::StringPropertyWidget(StringProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _lineEdit(0)
    {
        _lineEdit = new QLineEdit(QString::fromStdString(property->getValue()));
        
        addWidget(_lineEdit);

        connect(_lineEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(onTextChanged(const QString&)));
    }

    StringPropertyWidget::~StringPropertyWidget() {

    }

    void StringPropertyWidget::updateWidgetFromProperty() {
        StringProperty* prop = static_cast<StringProperty*>(_property);
        QString qs = QString::fromStdString(prop->getValue());
        if (_lineEdit->text() != qs) {
            _lineEdit->blockSignals(true);
            _lineEdit->setText(qs);
            _lineEdit->blockSignals(false);
        }
    }

    void StringPropertyWidget::onTextChanged(const QString& text) {
        StringProperty* prop = static_cast<StringProperty*>(_property);
        prop->setValue(text.toStdString());
    }
}