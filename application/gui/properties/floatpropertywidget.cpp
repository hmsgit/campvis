// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "floatpropertywidget.h"

namespace campvis {
    FloatPropertyWidget::FloatPropertyWidget(FloatProperty* property, DataContainer* dataContainer, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, dataContainer, parent)
        , _adjuster(0)
    {
        _adjuster = new DoubleAdjusterWidget();
        _adjuster->setMinimum(property->getMinValue());
        _adjuster->setMaximum(property->getMaxValue());
        _adjuster->setDecimals(property->getDecimals());
        _adjuster->setSingleStep(property->getStepValue());
        _adjuster->setValue(property->getValue());

        addWidget(_adjuster);

        connect(_adjuster, SIGNAL(valueChanged(double)), this, SLOT(onAdjusterValueChanged(double)));
        property->s_minMaxChanged.connect(this, &FloatPropertyWidget::onPropertyMinMaxChanged);
        property->s_stepChanged.connect(this, &FloatPropertyWidget::onPropertyStepChanged);
        property->s_decimalsChanged.connect(this, &FloatPropertyWidget::onPropertyDecimalsChanged);
    }

    FloatPropertyWidget::~FloatPropertyWidget() {
        FloatProperty* property = static_cast<FloatProperty*>(_property);

        property->s_minMaxChanged.disconnect(this);
        property->s_stepChanged.disconnect(this);
        property->s_decimalsChanged.disconnect(this);
    }

    void FloatPropertyWidget::updateWidgetFromProperty() {
        FloatProperty* prop = static_cast<FloatProperty*>(_property);
        _adjuster->blockSignals(true);
        if (static_cast<float>(_adjuster->value()) != prop->getValue())
            _adjuster->setValue(prop->getValue());
        if (static_cast<float>(_adjuster->minimum()) != prop->getMinValue())
            _adjuster->setMinimum(prop->getMinValue());
        if (static_cast<float>(_adjuster->maximum()) != prop->getMaxValue())
            _adjuster->setMaximum(prop->getMaxValue());
        if (static_cast<float>(_adjuster->singleStep()) != prop->getStepValue())
            _adjuster->setSingleStep(prop->getStepValue());
        if (_adjuster->decimals() != prop->getDecimals())
            _adjuster->setDecimals(prop->getDecimals());
        _adjuster->blockSignals(false);
    }

    void FloatPropertyWidget::onAdjusterValueChanged(double value) {
        ++_ignorePropertyUpdates;
        FloatProperty* prop = static_cast<FloatProperty*>(_property);
        prop->setValue(value);
        --_ignorePropertyUpdates;
    }

    void FloatPropertyWidget::onPropertyMinMaxChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0)
            emit s_propertyChanged(property);
    }

    void FloatPropertyWidget::onPropertyStepChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0)
            emit s_propertyChanged(property);
    }

    void FloatPropertyWidget::onPropertyDecimalsChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0)
            emit s_propertyChanged(property);
    }

    Vec2PropertyWidget::Vec2PropertyWidget(PropertyType* property, DataContainer* dataContainer /*= nullptr*/, QWidget* parent /*= 0*/) : VecPropertyWidget<2>(property, dataContainer, parent) {
        for (size_t i = 0; i < size; ++i) {
            connect(_adjusters[i], SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
        }
    }

    Vec3PropertyWidget::Vec3PropertyWidget(PropertyType* property, DataContainer* dataContainer /*= nullptr*/, QWidget* parent /*= 0*/) : VecPropertyWidget<3>(property, dataContainer, parent) {
        for (size_t i = 0; i < size; ++i) {
            connect(_adjusters[i], SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
        }
    }

    Vec4PropertyWidget::Vec4PropertyWidget(PropertyType* property, DataContainer* dataContainer /*= nullptr*/, QWidget* parent /*= 0*/) : VecPropertyWidget<4>(property, dataContainer, parent) {
        for (size_t i = 0; i < size; ++i) {
            connect(_adjusters[i], SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
        }
    }

}
