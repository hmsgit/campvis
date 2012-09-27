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

#include "tfgeometry.h"

#include "tgt/assert.h"
#include "tgt/texture.h"
#include "tgt/tgt_math.h"

#include <algorithm>

namespace {
    tgt::col4 toCol(const tgt::vec4& c) {
        return tgt::col4(
            static_cast<uint8_t>(255 * c.r), 
            static_cast<uint8_t>(255 * c.g), 
            static_cast<uint8_t>(255 * c.b), 
            static_cast<uint8_t>(255 * c.a));
    }

    tgt::vec4 toVec(const tgt::col4& c) {
        return tgt::vec4(
            static_cast<float>(c.r) / 255.f, 
            static_cast<float>(c.g) / 255.f, 
            static_cast<float>(c.b) / 255.f, 
            static_cast<float>(c.a) / 255.f);
    }
}

namespace TUMVis {

    bool operator< (const TFGeometry::KeyPoint& left, const TFGeometry::KeyPoint& right) {
        return left._position < right._position;
    }
    
    TFGeometry::TFGeometry(const std::vector<KeyPoint>& keyPoints)
        : _keyPoints(keyPoints)
    {
        std::sort(_keyPoints.begin(), _keyPoints.end());
    }

    TFGeometry::~TFGeometry() {

    }

    void TFGeometry::rasterize(tgt::Texture& texture) const {
        if (_keyPoints.size() < 2)
            return;

        int width = texture.getWidth();
        float rcpWidth = 1.f / static_cast<float>(width);

        // _keyPoints has at least 2 items
        std::vector<KeyPoint>::const_iterator start = _keyPoints.begin();
        std::vector<KeyPoint>::const_iterator end = _keyPoints.begin()+1;

        for (/* already inited */; end != _keyPoints.end(); ++start, ++end) {
            size_t startIndex = static_cast<size_t>(tgt::round(start->_position * width));
            size_t endIndex = static_cast<size_t>(tgt::round(end->_position * width));
            float dist = end->_position - start->_position;
            tgt::vec4 startColor = toVec(start->_color);
            tgt::vec4 endColor = toVec(end->_color);

            for (size_t i = startIndex; i < endIndex; ++i) {
                tgt::vec4 result = toVec(texture.texel<tgt::col4>(i));
                tgt::vec4 color = tgt::mix(startColor, endColor, tgt::clamp((static_cast<float>(i) * rcpWidth - start->_position) / dist, 0.f, 1.f));
                result = tgt::vec4(tgt::mix(color.xyz(), result.xyz(), result.a), result.a + (1.f - result.a) * color.a);
                texture.texel<tgt::col4>(i) = toCol(result);
            }

        }
    }

    void TFGeometry::render() const {
        // TODO: get rid of intermediade mode?
        glBegin(GL_POLYGON);

        if (_keyPoints.front()._color.w > 0) {
            glColor3ubv(_keyPoints.front()._color.elem);
            glVertex2f(_keyPoints.front()._position, 0.f);
        }

        for (std::vector<KeyPoint>::const_iterator it = _keyPoints.begin(); it != _keyPoints.end(); ++it) {
            glColor3ubv(it->_color.elem);
            float y = 255.f / static_cast<float>(it->_color.a);
            glVertex2f(it->_position, y);
        }

        if (_keyPoints.back()._color.w > 0) {
            glColor3ubv(_keyPoints.back()._color.elem);
            glVertex2f(_keyPoints.back()._position, 0.f);
        }

        glEnd();
    }

    TFGeometry* TFGeometry::createQuad(const tgt::vec2& interval, const tgt::col4& leftColor, const tgt::vec4 rightColor) {
        tgtAssert(interval.x >= 0.f && interval.y <= 1.f, "Interval out of bounds");

        std::vector<KeyPoint> keyPoints;
        keyPoints.push_back(KeyPoint(interval.x, leftColor));
        keyPoints.push_back(KeyPoint(interval.y, rightColor));
        return new TFGeometry(keyPoints);
    }


}