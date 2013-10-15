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

#ifndef ABSTRACTTRANSFERFUNCTIONEDITOR_H__
#define ABSTRACTTRANSFERFUNCTIONEDITOR_H__

#include "sigslot/sigslot.h"
#include <QBoxLayout>
#include <QLabel>
#include <QWidget>

namespace campvis {
    class AbstractProperty;
    class AbstractTransferFunction;

    /**
     * Abstract base class for transfer function editors.
     */
    class AbstractTransferFunctionEditor : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the AbstractTransferFunction \a tf.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        AbstractTransferFunctionEditor(AbstractTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~AbstractTransferFunctionEditor();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty() = 0;

        AbstractTransferFunction* _transferFunction;    ///< The transfer function this widget handles

        /// Semaphore acts as flag whether the widget shall ignore incoming signals from properties being updated.
        tbb::atomic<int> _ignorePropertyUpdates;

    private:
        /// Slot getting called when the transfer function has changed, so that the widget can be updated.
        virtual void onTFChanged();
    };
}

#endif // ABSTRACTTRANSFERFUNCTIONEDITOR_H__
