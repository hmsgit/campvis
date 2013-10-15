// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#include "propertycollectionwidget.h"

#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertywidgetfactory.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/abstractproperty.h"
#include "core/properties/propertycollection.h"

#include <QPushButton>

namespace campvis {


    PropertyCollectionWidget::PropertyCollectionWidget(QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _propCollection(0)
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
        
        // create widgets for the new PropertyCollection
        if (propertyCollection != 0) {
            for (std::vector<AbstractProperty*>::const_iterator it = propertyCollection->getProperties().begin(); it != propertyCollection->getProperties().end(); ++it) {
                QWidget* propWidget = PropertyWidgetFactory::createWidget(*it, dc);
                if (propWidget == 0)
                    propWidget = new QPushButton(QString::fromStdString((*it)->getTitle()));

                _widgetMap.insert(std::make_pair(*it, propWidget));
                _layout->addWidget(propWidget);
                propWidget->setVisible((*it)->isVisible());
                (*it)->s_visibilityChanged.connect(this, &PropertyCollectionWidget::onPropertyVisibilityChanged);
            }
        }
    }

    void PropertyCollectionWidget::setupWidget() {
        _layout = new QVBoxLayout();
        _layout->setSpacing(8);
        _layout->setMargin(0);
        setLayout(_layout);
    }

    void PropertyCollectionWidget::clearWidgetMap() {
        for (std::map<AbstractProperty*, QWidget*>::iterator it = _widgetMap.begin(); it != _widgetMap.end(); ++it) {
            it->first->s_visibilityChanged.disconnect(this);
            _layout->removeWidget(it->second);
            delete it->second;
        }

        _widgetMap.clear();
    }

    void PropertyCollectionWidget::onPropertyVisibilityChanged(const AbstractProperty* prop) {
        // const_cast does not harm anything.
        std::map<AbstractProperty*, QWidget*>::iterator item = _widgetMap.find(const_cast<AbstractProperty*>(prop));
        if (item != _widgetMap.end())
            item->second->setVisible(item->first->isVisible());
    }

}