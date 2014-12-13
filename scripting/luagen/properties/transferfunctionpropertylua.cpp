// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "transferfunctionpropertylua.h"

//#include "transferfunctionluafactory.h"
#include "core/tools/stringutils.h"
#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/geometry2dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/classification/tfgeometry2d.h"

namespace campvis {
    TransferFunctionPropertyLua::TransferFunctionPropertyLua(TransferFunctionProperty* property)
        : AbstractPropertyLua(property)     {
    }

    TransferFunctionPropertyLua::~TransferFunctionPropertyLua() {
    }

    std::string TransferFunctionPropertyLua::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
        TransferFunctionProperty* prop = static_cast<TransferFunctionProperty*>(_property);
        AbstractTransferFunction* tf = prop->getTF();
        const cgt::vec2& domain = tf->getIntensityDomain();

        std::string ret = "";

        if (SimpleTransferFunction* tester = dynamic_cast<SimpleTransferFunction*>(tf)) {
            // dvrTF = ...
            ret += "tf = campvis.SimpleTransferFunction(" + StringUtils::toString((float)tf->getSize().x)
                +", cgt.vec2("+StringUtils::toString((float)domain.x) +", " + StringUtils::toString((float)domain.y) + "))\n";

            ret += "tf:setLeftColor(cgt.col4(" + StringUtils::toString((float)tester->getLeftColor().r)
                + ", " + StringUtils::toString((float)tester->getLeftColor().g)
                + ", " + StringUtils::toString((float)tester->getLeftColor().b)
                + ", " + StringUtils::toString((float)tester->getLeftColor().a) + "))\n";

            ret += "tf:setRightColor(cgt.col4(" + StringUtils::toString((float)tester->getRightColor().r)
                + ", " + StringUtils::toString((float)tester->getRightColor().g)
                + ", " + StringUtils::toString((float)tester->getRightColor().b)
                + ", " + StringUtils::toString((float)tester->getRightColor().a) + "))\n";
        }

        if (Geometry1DTransferFunction* tester = dynamic_cast<Geometry1DTransferFunction*>(tf)) {
            // dvrTF = ...
            ret += "tf = campvis.Geometry1DTransferFunction(" + StringUtils::toString((float)tf->getSize().x)
                +", cgt.vec2("+StringUtils::toString((float)domain.x) +", " + StringUtils::toString((float)domain.y) + "))\n";

            const std::vector<TFGeometry1D*>& _geometries = tester->getGeometries();
            for (size_t i = 0; i < _geometries.size(); i++) {
                std::vector<TFGeometry1D::KeyPoint>& kp = _geometries[i]->getKeyPoints();
                cgtAssert(kp.size() >= 2, "There should be at least two key points");
                float x = kp[0]._position;
                float y = kp[1]._position;
                cgt::col4 lc = kp[0]._color;
                cgt::col4 rc = kp[1]._color;

                // geometry = ...
                ret += "geometry = campvis.TFGeometry1D_createQuad(cgt.vec2("+ StringUtils::toString(x)
                    + "," + StringUtils::toString(y) + "), " 

                    + "cgt.col4(" + StringUtils::toString((float)lc.r) + ", "
                    + StringUtils::toString((float)lc.g) + ", "
                    + StringUtils::toString((float)lc.b) + ", "
                    + StringUtils::toString((float)lc.a) + "), "

                    + "cgt.col4(" + StringUtils::toString((float)rc.r) + ", "
                    + StringUtils::toString((float)rc.g) + ", "
                    + StringUtils::toString((float)rc.b) + ", "
                    + StringUtils::toString((float)rc.a) + "))\n";

                for (size_t i = 2; i < kp.size(); i++) {
                    ret += "geometry:addKeyPoint(" + StringUtils::toString(kp[i]._position)
                        + ", cgt.col4(" + StringUtils::toString((float)kp[i]._color.r) + ", "
                        + StringUtils::toString((float)kp[i]._color.g) + ", "
                        + StringUtils::toString((float)kp[i]._color.b) + ", "
                        + StringUtils::toString((float)kp[i]._color.a) + "))\n";
                }
                // dvrTF.addGeo ...
                ret += "tf:addGeometry(geometry)\n";
            }
        }

        if (Geometry2DTransferFunction* tester = dynamic_cast<Geometry2DTransferFunction*>(tf)) {
            //TODO: fix me when 2D geometry is fully implemented
            cgtAssert(false, "Fix Geometry2DTransferFunction lua scripting first");
            ret += "tf = campvis.Geometry2DTransferFunction(" + StringUtils::toString(tf->getSize().x)
                +", cgt.vec2("+StringUtils::toString(domain.x) +", " + StringUtils::toString(domain.y) + "))\n";

            const std::vector<TFGeometry2D*>& _geometries = tester->getGeometries();
            for (size_t i = 0; i < _geometries.size(); i++) {
                std::vector<TFGeometry2D::KeyPoint>& kp = _geometries[i]->getKeyPoints();
                cgtAssert(kp.size() >= 4, "There should be at least four key points");
                cgt::vec2 ll = kp[0]._position;
                cgt::vec2 ur = kp[2]._position;
                cgt::col4 col = kp[0]._color;

                ret += "geometry = campvis.TFGeometry1D_createQuad(cgt.vec2("+ StringUtils::toString(ll.x)
                    + "," + StringUtils::toString(ll.y) + "), " 

                    + "cgt.vec2(" + StringUtils::toString(ur.x) + ", "
                    + StringUtils::toString(ur.y) + "), "

                    + "cgt.col4(" + StringUtils::toString((float)col.r) + ", "
                    + StringUtils::toString((float)col.g) + ", "
                    + StringUtils::toString((float)col.b) + ", "
                    + StringUtils::toString((float)col.a) + "))\n";
                ret += "tf:addGeometry(geometry)\n";
            }
        }

        // replaceTF
        ret += luaProc;
        ret += "getNestedProperty(\"" + propNamePrefix + _property->getName() + "\"):replaceTF(tf)\n";

        return ret;
    }

}

