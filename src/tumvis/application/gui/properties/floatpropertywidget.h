// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef FLOATPROPERTYWIDGET_H__
#define FLOATPROPERTYWIDGET_H__

#include "application/gui/properties/abstractpropertywidget.h"
#include "core/properties/numericproperty.h"

#include <QDoubleSpinBox>

namespace TUMVis {
    /**
     * Widget for a IntProperty
     */
    class FloatPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new FloatPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        FloatPropertyWidget(FloatProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~FloatPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onValueChanged(double value);

    private:
        /// Slot getting called when the property's min or max value has changed, so that the widget can be updated.
        virtual void onPropertyMinMaxChanged(const AbstractProperty* property);

        QDoubleSpinBox* _spinBox;

    };
}

#endif // FLOATPROPERTYWIDGET_H__
