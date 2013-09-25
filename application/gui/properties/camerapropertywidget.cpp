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

#include "camerapropertywidget.h"

#include <QGridLayout>
#include <QLabel>
#include "core/tools/stringutils.h"

namespace campvis {
    CameraPropertyWidget::CameraPropertyWidget(CameraProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, parent)
        , _lblCameraPosition(0)
        , _lblLookDirection(0)
        , _lblUpVector(0)
    {
        _widget = new QWidget(this);
        QGridLayout* gridLayout = new QGridLayout(_widget);
        _widget->setLayout(gridLayout);

        _lblCameraPosition = new QLabel("Position: ", _widget);
        gridLayout->addWidget(_lblCameraPosition, 0, 0);
        _lblFocusPosition = new QLabel("Focus: ", _widget);
        gridLayout->addWidget(_lblFocusPosition, 1, 0);
        _lblLookDirection = new QLabel("Look Direction: ", _widget);
        gridLayout->addWidget(_lblLookDirection, 2, 0);
        _lblUpVector = new QLabel("Up Vector: ", _widget);
        gridLayout->addWidget(_lblUpVector, 3, 0);

        addWidget(_widget);
        updateWidgetFromProperty();
    }

    CameraPropertyWidget::~CameraPropertyWidget() {
    }

    void CameraPropertyWidget::updateWidgetFromProperty() {
        CameraProperty* prop = static_cast<CameraProperty*>(_property);
        _lblCameraPosition->setText("Position: " + QString::fromStdString(StringUtils::toString(prop->getValue().getPosition())));
        _lblFocusPosition->setText("Focus: " + QString::fromStdString(StringUtils::toString(prop->getValue().getFocus())));
        _lblLookDirection->setText("Look Direction: " + QString::fromStdString(StringUtils::toString(prop->getValue().getLook())));
        _lblUpVector->setText("Up Vector: " + QString::fromStdString(StringUtils::toString(prop->getValue().getUpVector())));
    }

}