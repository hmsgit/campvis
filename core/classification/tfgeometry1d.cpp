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

#include "tfgeometry1d.h"

#include "tgt/assert.h"
#include "tgt/texture.h"
#include "tgt/tgt_math.h"

#include <algorithm>

namespace campvis {

    bool operator< (const TFGeometry1D::KeyPoint& left, const TFGeometry1D::KeyPoint& right) {
        return left._position < right._position;
    }
    
    TFGeometry1D::TFGeometry1D(const std::vector<KeyPoint>& keyPoints)
        : _keyPoints(keyPoints)
    {
        std::sort(_keyPoints.begin(), _keyPoints.end());
    }

    TFGeometry1D::~TFGeometry1D() {

    }

    std::vector<TFGeometry1D::KeyPoint>& TFGeometry1D::getKeyPoints() {
        return _keyPoints;
    }

    void TFGeometry1D::renderIntoEditor() const {
        // TODO: get rid of intermediade mode?
        if (_keyPoints.size() < 2)
            return;

        glBegin(GL_QUADS);
        std::vector<KeyPoint>::const_iterator a = _keyPoints.begin();
        std::vector<KeyPoint>::const_iterator b = _keyPoints.begin()+1;
        for (/* already inited */; b != _keyPoints.end(); ++a, ++b) {
            glColor4ubv(a->_color.elem);
            float y = static_cast<float>(a->_color.a) / 255.f;
            glVertex2f(a->_position, 0.f);
            glVertex2f(a->_position, y);

            glColor4ubv(b->_color.elem);
            y = static_cast<float>(b->_color.a) / 255.f;
            glVertex2f(b->_position, y);
            glVertex2f(b->_position, 0.f);
        }
        glEnd();
    }

    void TFGeometry1D::render() const {
        // TODO: get rid of intermediade mode?
        if (_keyPoints.size() < 2)
            return;

        glBegin(GL_QUADS);
        std::vector<KeyPoint>::const_iterator a = _keyPoints.begin();
        std::vector<KeyPoint>::const_iterator b = _keyPoints.begin()+1;
        for (/* already inited */; b != _keyPoints.end(); ++a, ++b) {
            glColor4ubv(a->_color.elem);
            glVertex2f(a->_position, 0.f);
            glVertex2f(a->_position, 1.f);

            glColor4ubv(b->_color.elem);
            glVertex2f(b->_position, 1.f);
            glVertex2f(b->_position, 0.f);
        }
        glEnd();
    }
    TFGeometry1D* TFGeometry1D::createQuad(const tgt::vec2& interval, const tgt::col4& leftColor, const tgt::vec4& rightColor) {
        tgtAssert(interval.x >= 0.f && interval.y <= 1.f, "Interval out of bounds");

        std::vector<KeyPoint> keyPoints;
        keyPoints.push_back(KeyPoint(interval.x, leftColor));
        keyPoints.push_back(KeyPoint(interval.y, rightColor));
        return new TFGeometry1D(keyPoints);
    }


}