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

#ifndef PROPERTYCOLLECTIONWIDGET_H__
#define PROPERTYCOLLECTIONWIDGET_H__

#include <QList>
#include <QVBoxLayout>
#include <QWidget>

namespace TUMVis {
    class HasPropertyCollection;

    /**
     * Main Window for the TUMVis application.
     * Wraps a nice Qt GUI around the TumVisApplication instance given during creation.
     */
    class PropertyCollectionWidget : public QWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyCollectionWidget.
         * \param   parent  Parent widget, may be 0.
         */
        PropertyCollectionWidget(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~PropertyCollectionWidget();

    public slots:
        /**
         * Updates the property collection this widget works on.
         * \param   propertyCollection  New HasPropertyCollection instance for this widget, may be 0.
         */
        void updatePropCollection(HasPropertyCollection* propertyCollection);


    private:
        /**
         * Sets up this widget
         */
        void setupWidget();

        HasPropertyCollection* _propCollection;    ///< The HasPropertyCollection instance this widget is currently working on.
        QVBoxLayout* _layout;
        QList<QWidget*> _widgetList;
    };
}

#endif // PROPERTYCOLLECTIONWIDGET_H__
