// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
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

#include "boolpropertywidget.h"

#include <QCheckBox>

namespace campvis {
    BoolPropertyWidget::BoolPropertyWidget(BoolProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _spinBox(0)
    {
        _spinBox = new QCheckBox(this);
        _spinBox->setChecked(property->getValue());
        
        addWidget(_spinBox);

        connect(_spinBox, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    }

    BoolPropertyWidget::~BoolPropertyWidget() {
    }

    void BoolPropertyWidget::updateWidgetFromProperty() {
        BoolProperty* prop = static_cast<BoolProperty*>(_property);
        _spinBox->blockSignals(true);
        _spinBox->setChecked(prop->getValue());
        _spinBox->blockSignals(false);
    }

    void BoolPropertyWidget::onStateChanged(int  value) {
        _ignorePropertyUpdates = true;
        BoolProperty* prop = static_cast<BoolProperty*>(_property);
        prop->setValue(_spinBox->isChecked());
        _ignorePropertyUpdates = false;
    }

}