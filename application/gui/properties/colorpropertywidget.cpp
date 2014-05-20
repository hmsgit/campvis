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

#include "colorpropertywidget.h"
#include "core/properties/colorproperty.h"

namespace campvis {
    ColorPropertyWidget::ColorPropertyWidget(ColorProperty* property, DataContainer* dataContainer, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, dataContainer, parent)
        //, _lblColorPicker(0)
        , _colorPicker(0)
    {
        //_lblColorPicker = new QLabel("Change color: ", this);
        _colorPicker = new ColorPickerWidget(QtColorTools::toQColor(property->getValue()), this);

        addWidget(_colorPicker);

        //connect(_colorPicker, SIGNAL(valueChanged(tgt::vec4)), this, SLOT(onColorChanged(tgt::vec4)));
        connect(_colorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(onColorChanged(const QColor&)));

    }

    ColorPropertyWidget::~ColorPropertyWidget() {
        ColorProperty* property = static_cast<ColorProperty*>(_property);
    }

    void ColorPropertyWidget::updateWidgetFromProperty() {
        ColorProperty* prop = static_cast<ColorProperty*>(_property);
        _colorPicker->blockSignals(true);
        const tgt::vec4 val = static_cast<ColorProperty*>(_property)->getValue();
        _colorPicker->setColor(QtColorTools::toQColor(tgt::vec4(val.x *255, val.y*255, val.z*255, val.w*255)));
        _colorPicker->blockSignals(false);
    }

    void ColorPropertyWidget::onColorChanged(const QColor& value) {
        ++_ignorePropertyUpdates;
        ColorProperty* prop = static_cast<ColorProperty*>(_property);
        tgt::vec4 val = QtColorTools::toTgtColor(_colorPicker->color());
        prop->setValue(tgt::vec4(val.x/val.w, val.y/val.w, val.z/val.w, val.w/val.w));
        --_ignorePropertyUpdates;
    }

    void ColorPropertyWidget::onPropertyMinMaxChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0)
            emit s_propertyChanged(property);
    }

}
