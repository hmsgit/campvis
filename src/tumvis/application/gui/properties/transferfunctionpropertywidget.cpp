// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "transferfunctionpropertywidget.h"

#include "application/gui/properties/abstracttransferfunctioneditor.h"
#include "application/gui/properties/transferfunctioneditorfactory.h"
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

namespace TUMVis {
    TransferFunctionPropertyWidget::TransferFunctionPropertyWidget(TransferFunctionProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _widget(0)
        , _gridLayout(0)
        , _lblDomain(0)
        , _spinDomainLeft(0)
        , _spinDomainRight(0)
        , _btnEditTF(0)
        , _dockWidget(0)
        , _editor(0)
    {
        _widget = new QWidget(this);
        _gridLayout = new QGridLayout(_widget);
        _widget->setLayout(_gridLayout);

        _lblDomain = new QLabel("Intensity Domain: ", _widget);
        _gridLayout->addWidget(_lblDomain, 0, 0);


        _spinDomainLeft = new QDoubleSpinBox(_widget);
        _spinDomainLeft->setMinimum(0);
        _spinDomainLeft->setDecimals(2);
        _spinDomainLeft->setSingleStep(0.01);
        _gridLayout->addWidget(_spinDomainLeft, 0, 1);

        _spinDomainRight = new QDoubleSpinBox(_widget);
        _spinDomainRight->setMaximum(1);
        _spinDomainRight->setDecimals(2);
        _spinDomainRight->setSingleStep(0.01);
        _gridLayout->addWidget(_spinDomainRight, 0, 2);

        updateWidgetFromProperty();

        _btnEditTF = new QPushButton("Edit Transfer Function", _widget);
        _gridLayout->addWidget(_btnEditTF, 1, 0, 1, 3);

        addWidget(_widget);

        connect(_spinDomainLeft, SIGNAL(valueChanged(double)), this, SLOT(onDomainChanged(double)));
        connect(_spinDomainRight, SIGNAL(valueChanged(double)), this, SLOT(onDomainChanged(double)));
        connect(_btnEditTF, SIGNAL(clicked(bool)), this, SLOT(onEditClicked(bool)));
    }

    TransferFunctionPropertyWidget::~TransferFunctionPropertyWidget() {
        delete _dockWidget;
    }

    void TransferFunctionPropertyWidget::updateWidgetFromProperty() {
        TransferFunctionProperty* prop = static_cast<TransferFunctionProperty*>(_property);
        AbstractTransferFunction* tf = prop->getTF();
        const tgt::vec2& domain = tf->getIntensityDomain();

        _spinDomainLeft->blockSignals(true);
        _spinDomainLeft->setMaximum(domain.y);
        _spinDomainLeft->setValue(domain.x);
        _spinDomainLeft->blockSignals(false);

        _spinDomainRight->blockSignals(true);
        _spinDomainRight->setMinimum(domain.x);
        _spinDomainRight->setValue(domain.y);
        _spinDomainRight->blockSignals(false);
    }

    void TransferFunctionPropertyWidget::onDomainChanged(double value) {
        TransferFunctionProperty* prop = static_cast<TransferFunctionProperty*>(_property);
        _ignorePropertyUpdates = true;
        _spinDomainLeft->setMaximum(_spinDomainRight->value());
        _spinDomainRight->setMinimum(_spinDomainLeft->value());
        tgt::vec2 newDomain(static_cast<float>(_spinDomainLeft->value()), static_cast<float>(_spinDomainRight->value()));
        prop->getTF()->setIntensityDomain(newDomain);
        _ignorePropertyUpdates = false;
    }

    void TransferFunctionPropertyWidget::onEditClicked(bool checked) {
        if (_editor == 0) {
            TransferFunctionProperty* prop = static_cast<TransferFunctionProperty*>(_property);
            _editor = TransferFunctionEditorFactory::createEditor(prop->getTF());

            _dockWidget = new QDockWidget("Transfer Function Editor");
            _dockWidget->setWidget(_editor);
        }

        _dockWidget->setVisible(true);
    }


}