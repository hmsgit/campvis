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

#ifndef DATANAMEPROPERTYWIDGET_H__
#define DATANAMEPROPERTYWIDGET_H__

#include "sigslot/sigslot.h"
#include "application/gui/properties/abstractpropertywidget.h"

class QComboBox;
class QLineEdit;

namespace campvis {
    class DataNameProperty;
    class DataContainer;
    class DataHandle;

    /**
     * Widget for a DataNameProperty
     */
    class DataNamePropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT

    public:
        /**
         * Creates a new PropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        DataNamePropertyWidget(DataNameProperty* property, DataContainer* dc, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~DataNamePropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

        void onDataAdded(const std::string& key, const DataHandle& dh);

    private slots:
        void onTextChanged(const QString& text);

    private:
        /**
         * Set the combo box's current text to the provided value.
         *
         * If \c text can already be found in the combo box, the corresponding item is made active. Otherwise, \c text
         * is set as the contents of the combo box's text edit.
         *
         * \param   text      text to be set in the combo box
         */
        void setCurrentComboBoxText(const QString& text);

        QLineEdit* _lineEdit;
        QComboBox* _combobox;
        DataContainer* _dc;

    };
}

#endif // DATANAMEPROPERTYWIDGET_H__
