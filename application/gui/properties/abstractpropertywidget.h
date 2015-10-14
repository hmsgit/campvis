// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef ABSTRACTPROPERTYWIDGET_H__
#define ABSTRACTPROPERTYWIDGET_H__

#include "sigslot/sigslot.h"
#include "tbb/atomic.h"

#include "application/applicationapi.h"

#include <QBoxLayout>
#include <QLabel>
#include <QWidget>

namespace campvis {
    class AbstractProperty;
    class DataContainer;

    /**
     * Abstract base class for property widgets.
     */
    class CAMPVIS_APPLICATION_API AbstractPropertyWidget : public QWidget, public sigslot::has_slots {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyWidget for the property \a property.
         *
         * If displayBoxed is true, the widget is displayed vertically in a QGroupBox.
         *
         * \param   property        The property the widget shall handle.
         * \param   displayBoxed    Should the widget be displayed in a group box?
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr. However, some derived classed might need a valid pointer here!
         * \param   parent          Parent Qt widget, defaults to nullptr.
         */
        AbstractPropertyWidget(AbstractProperty* property, bool displayBoxed = false, DataContainer* dataContainer = nullptr, QWidget* parent = nullptr);

        /**
         * Destructor
         */
        virtual ~AbstractPropertyWidget();

        /**
         * Shows or hides the label that appears on the left hand side of the widget.
         *
         * \param   isVisible       Wether the label should be shown or not
         */
        void setLabelVisibile(bool isVisible);

    protected:
        /**
         * Adds a widget to the local Qt layout.
         */
        void addWidget(QWidget* widget, int stretch = 1);

        AbstractProperty* _property;    ///< The property this widget handles
        DataContainer* _dataContainer;  ///< DataContainer to use (e.g. to populate GUI), may be 0!

        /// Semaphore acts as flag whether the widget shall ignore incoming signals from properties being updated.
        tbb::atomic<int> _ignorePropertyUpdates;

    protected slots:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty() {};

    signals:
        /**
         * Internal signal used to update the property widget in a thread-safe way.
         *
         * This class' \ref onPropertyChanged() slot is invoked from non-GUI threads. As a result,
         * \ref updateWidgetFromProperty() couldn't access any Qt widgets safely if it was called from there directly,
         * because it would execute in a non-GUI thread. However, if we invoke \ref updateWidgetFromProperty() via
         * a signal-slot connection with \ref s_propertyChanged(), Qt takes care of queueing slot accesses in the GUI
         * thread for us.
         */
        void s_propertyChanged(const AbstractProperty* property);

    private:
        /// Slot getting called when the property has changed, so that the widget can be updated.
        virtual void onPropertyChanged(const AbstractProperty* property);

        QBoxLayout* _layout;
        QLabel* _titleLabel;
    };
}

#endif // ABSTRACTPROPERTYWIDGET_H__
