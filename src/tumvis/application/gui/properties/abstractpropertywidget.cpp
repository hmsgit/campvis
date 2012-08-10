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

#include "abstractpropertywidget.h"
#include "core/properties/abstractproperty.h"

namespace TUMVis {

    AbstractPropertyWidget::AbstractPropertyWidget(AbstractProperty* property, QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _property(property)
        , _ignorePropertyUpdates(false)
        , _layout(0)
    {
        _titleLabel = new QLabel(QString::fromStdString(_property->getTitle() + ":"), this);

        _layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
        _layout->setSpacing(16);
        _layout->addWidget(_titleLabel, 0);

        _property->s_changed.connect(this, &AbstractPropertyWidget::onPropertyChanged);
    }

    AbstractPropertyWidget::~AbstractPropertyWidget() {
        _property->s_changed.disconnect(this);
    }

    void AbstractPropertyWidget::addWidget(QWidget* widget) {
        _layout->addWidget(widget, 1);
    }

    void AbstractPropertyWidget::onPropertyChanged(const AbstractProperty* property) {
        if (!_ignorePropertyUpdates)
            updateWidgetFromProperty();
    }
}