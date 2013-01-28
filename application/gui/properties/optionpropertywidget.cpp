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

#include "optionpropertywidget.h"

#include <QComboBox>
#include "core/properties/optionproperty.h"

namespace campvis {
    OptionPropertyWidget::OptionPropertyWidget(AbstractOptionProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _comboBox(0)
    {
        _comboBox = new QComboBox(this);
        updateWidgetFromProperty();
        addWidget(_comboBox);

        connect(_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndexChanged(int)));
    }

    OptionPropertyWidget::~OptionPropertyWidget() {
        static_cast<IntProperty*>(_property)->s_minMaxChanged.disconnect(this);
    }

    void OptionPropertyWidget::updateWidgetFromProperty() {
        AbstractOptionProperty* prop = static_cast<AbstractOptionProperty*>(_property);
        _comboBox->blockSignals(true);
        _comboBox->clear();

        // build combo box from descriptions
        std::vector< std::pair<std::string, std::string> > options = prop->getOptionsAsPairOfStrings();
        for (std::vector< std::pair<std::string, std::string> >::iterator it = options.begin(); it != options.end(); ++it) {
            _comboBox->addItem(QString::fromStdString(it->second), QString::fromStdString(it->first));
        }

        // set selected options
        _comboBox->setCurrentIndex(prop->getValue());

        _comboBox->blockSignals(false);
    }

    void OptionPropertyWidget::onComboBoxIndexChanged(int value) {
        _ignorePropertyUpdates = true;
        AbstractOptionProperty* prop = static_cast<AbstractOptionProperty*>(_property);
        prop->setValue(value);
        _ignorePropertyUpdates = false;
    }

}