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

#include "abstractpropertywidget.h"
#include "core/properties/abstractproperty.h"

#include <QGroupBox>

namespace campvis {

    AbstractPropertyWidget::AbstractPropertyWidget(AbstractProperty* property, bool displayBoxed /*= false*/,
                                                   QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _property(property)
        , _layout(0)
    {
        _ignorePropertyUpdates = 0;

        if (displayBoxed) {
            QBoxLayout* outerLayout = new QBoxLayout(QBoxLayout::LeftToRight, this);
            QGroupBox* groupBox = new QGroupBox(QString::fromStdString(_property->getTitle()));

            outerLayout->setMargin(4);
            outerLayout->addWidget(groupBox);

            _layout = new QBoxLayout(QBoxLayout::TopToBottom, groupBox);
            _layout->setSpacing(1);
            _layout->setMargin(3);
        }
        else {
            _titleLabel = new QLabel(QString::fromStdString(_property->getTitle() + ":"), this);

            _layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
            _layout->setSpacing(8);
            _layout->setMargin(4);
            _layout->addWidget(_titleLabel, 0);
        }

        _property->s_changed.connect(this, &AbstractPropertyWidget::onPropertyChanged);
        // cppcheck-suppress pureVirtualCall
        connect(this, SIGNAL(s_propertyChanged(const AbstractProperty*)), this, SLOT(updateWidgetFromProperty()));
    }

    AbstractPropertyWidget::~AbstractPropertyWidget() {
        _property->s_changed.disconnect(this);
    }

    void AbstractPropertyWidget::addWidget(QWidget* widget) {
        _layout->addWidget(widget, 1);
    }

    void AbstractPropertyWidget::onPropertyChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0)
            emit s_propertyChanged(property);
    }
}
