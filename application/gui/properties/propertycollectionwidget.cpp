// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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