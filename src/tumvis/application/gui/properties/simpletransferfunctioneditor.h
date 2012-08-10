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

#ifndef SIMPLETRANSFERFUNCTIONEDITOR_H__
#define SIMPLETRANSFERFUNCTIONEDITOR_H__

#include "application/gui/properties/abstracttransferfunctioneditor.h"
#include <QWidget>

class QGridLayout;
class QLabel;

namespace TUMVis {
    class ColorPickerWidget;
    class SimpleTransferFunction;

    /**
     * Editor widget for a SimpleTransferFunction.
     */
    class SimpleTransferFunctionEditor : public AbstractTransferFunctionEditor {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the TransferFunctionProperty \a property.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        SimpleTransferFunctionEditor(SimpleTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~SimpleTransferFunctionEditor();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onLeftColorChanged(const QColor& color);
        void onRightColorChanged(const QColor& color);

    private:
        QGridLayout* _layout;

        QLabel* _lblColorLeft;
        QLabel* _lblColorRight;
        ColorPickerWidget* _cpwColorLeft;
        ColorPickerWidget* _cpwColorRight;
    };
}

#endif // SIMPLETRANSFERFUNCTIONEDITOR_H__
