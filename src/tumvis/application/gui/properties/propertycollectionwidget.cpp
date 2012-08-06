#include "propertycollectionwidget.h"

#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertywidgetfactory.h"
#include "core/properties/abstractproperty.h"
#include "core/properties/propertycollection.h"

#include <QPushButton>

namespace TUMVis {


    PropertyCollectionWidget::PropertyCollectionWidget(QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _propCollection(0)
        , _layout(0)
    {
        setupWidget();
    }

    PropertyCollectionWidget::~PropertyCollectionWidget() {
        qDeleteAll(_widgetList);
    }

    void PropertyCollectionWidget::updatePropCollection(HasPropertyCollection* propertyCollection) {
        // remove and delete all widgets of the previous PropertyCollection
        for (QList<QWidget*>::iterator it = _widgetList.begin(); it != _widgetList.end(); ++it) {
            _layout->removeWidget(*it);
        }
        qDeleteAll(_widgetList);
        _widgetList.clear();
        
        // create widgets for the new PropertyCollection
        for (std::vector<AbstractProperty*>::const_iterator it = propertyCollection->getProperties().begin(); it != propertyCollection->getProperties().end(); ++it) {
            QWidget* propWidget = PropertyWidgetFactory::createWidget(*it);
            if (propWidget == 0)
                propWidget = new QPushButton(QString::fromStdString((*it)->getTitle()));

            _widgetList.push_back(propWidget);
            _layout->addWidget(propWidget);
        }
    }

    void PropertyCollectionWidget::setupWidget() {
        _layout = new QVBoxLayout();
        setLayout(_layout);
    }

}