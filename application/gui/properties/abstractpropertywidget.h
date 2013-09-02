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

#ifndef ABSTRACTPROPERTYWIDGET_H__
#define ABSTRACTPROPERTYWIDGET_H__

#include "sigslot/sigslot.h"
#include "tbb/atomic.h"
#include <QBoxLayout>
#include <QLabel>
#include <QWidget>

namespace campvis {
    class AbstractProperty;

    /**
     * Abstract base class for property widgets.
     */
    class AbstractPropertyWidget : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyWidget for the property \a property.
         *
         * If displayBoxed is true, the widget is displayed vertically in a QGroupBox.
         *
         * \param   property        The property the widget shall handle
         * \param   displayBoxed    Should the widget be displayed in a group box?
         * \param   parent          Parent Qt widget
         */
        AbstractPropertyWidget(AbstractProperty* property, bool displayBoxed = false, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~AbstractPropertyWidget();


    public slots:

    protected:
        /**
         * Adds a widget to the local Qt layout.
         */
        void addWidget(QWidget* widget);

        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty() = 0;

        AbstractProperty* _property;    ///< The property this widget handles

        /// Semaphore acts as flag whether the widget shall ignore incoming signals from properties being updated.
        tbb::atomic<int> _ignorePropertyUpdates;

    private:
        /// Slot getting called when the property has changed, so that the widget can be updated.
        virtual void onPropertyChanged(const AbstractProperty* property);

        QBoxLayout* _layout;
        QLabel* _titleLabel;
    };
}

#endif // ABSTRACTPROPERTYWIDGET_H__
