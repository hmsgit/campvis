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

#include "propertycollectionwidget.h"

#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertywidgetfactory.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/abstractproperty.h"
#include "core/properties/propertycollection.h"

#include <QMetaType>
#include <QPushButton>

namespace campvis {
    PropertyCollectionWidget::PropertyCollectionWidget(QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _propCollection(0)
        , _dataContainer(0)
        , _layout(0)
    {
        setupWidget();
    }

    PropertyCollectionWidget::~PropertyCollectionWidget() {
        clearWidgetMap();
    }

    void PropertyCollectionWidget::updatePropCollection(HasPropertyCollection* propertyCollection, DataContainer* dc) {
        // remove and delete all widgets of the previous PropertyCollection
        clearWidgetMap();
        
        _propCollection = propertyCollection;
        _dataContainer = dc;

        // create widgets for the new PropertyCollection
        if (propertyCollection != 0) {
            for (std::vector<AbstractProperty*>::const_iterator it = propertyCollection->getProperties().begin(); it != propertyCollection->getProperties().end(); ++it) {
                addProperty(*it);
            } 

            propertyCollection->s_propertyAdded.connect(this, &PropertyCollectionWidget::onPropCollectionPropAdded);
            propertyCollection->s_propertyRemoved.connect(this, &PropertyCollectionWidget::onPropCollectionPropRemoved);
        }
        _layout->addStretch(1);
    }

    void PropertyCollectionWidget::setupWidget() {
        _layout = new QVBoxLayout();
        _layout->setSpacing(8);
        _layout->setMargin(0);
        setLayout(_layout);
        connect(this, SIGNAL(s_propertyVisibilityChanged(const AbstractProperty*)), this, SLOT(onWidgetVisibilityChanged(const AbstractProperty*)), Qt::QueuedConnection);
        connect(this, SIGNAL(propertyAdded(AbstractProperty*)), this, SLOT(addProperty(AbstractProperty*)));
        connect(this, SIGNAL(propertyRemoved(AbstractProperty*, QWidget*)), this, SLOT(removeProperty(AbstractProperty*, QWidget*)));
    }

    void PropertyCollectionWidget::clearWidgetMap() {
        for (std::map<AbstractProperty*, QWidget*>::iterator it = _widgetMap.begin(); it != _widgetMap.end(); ++it) {
            it->first->s_visibilityChanged.disconnect(this);
            removeProperty(it->first, it->second);
        }

        _widgetMap.clear();

        if (_propCollection != 0) {
            _propCollection->s_propertyAdded.disconnect(this);
            _propCollection->s_propertyRemoved.disconnect(this);
        }
    }

    void PropertyCollectionWidget::onPropertyVisibilityChanged(const AbstractProperty* prop) {
        // This method is not always called on the main thread, so it is not safe to check anything here.
        // The event is instead forwarded using the QT sigslot mechanism through a queued connection to the
        // main thread.

        emit s_propertyVisibilityChanged(prop);
    }

    void PropertyCollectionWidget::onWidgetVisibilityChanged(const AbstractProperty* prop) {
        // const_cast does not harm anything.
        std::map<AbstractProperty*, QWidget*>::iterator item = _widgetMap.find(const_cast<AbstractProperty*>(prop));
        if (item != _widgetMap.end())
            item->second->setVisible(item->first->isVisible());
    }

    void PropertyCollectionWidget::onPropCollectionPropAdded(AbstractProperty* prop) {
        emit propertyAdded(prop);
    }

    void PropertyCollectionWidget::onPropCollectionPropRemoved(AbstractProperty* prop) {
        std::map<AbstractProperty*, QWidget*>::iterator it = _widgetMap.find(prop);
        if (it != _widgetMap.end()) {
            emit propertyRemoved(it->first, it->second);
            _widgetMap.erase(it);
        }
    }

    void PropertyCollectionWidget::addProperty(AbstractProperty* prop) {
        QWidget* propWidget = PropertyWidgetFactory::getRef().createWidget(prop, _dataContainer);
        if (propWidget == 0)
            propWidget = new QPushButton(QString::fromStdString(prop->getTitle()));

        _widgetMap.insert(std::make_pair(prop, propWidget));
        _layout->addWidget(propWidget);

        prop->s_visibilityChanged.connect(this, &PropertyCollectionWidget::onPropertyVisibilityChanged);
        prop->s_visibilityChanged.emitSignal(prop);
    }

    void PropertyCollectionWidget::removeProperty(AbstractProperty* prop, QWidget* widget) {
        prop->s_visibilityChanged.disconnect(this);
        _layout->removeWidget(widget);
        delete widget;
    }

}