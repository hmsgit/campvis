/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2015               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "cgt/event/mouseevent.h"

namespace cgt {

MouseEvent::MouseEvent(int x, int y, MouseAction action, Event::Modifier mod, MouseButtons button, ivec2 viewport)
    : Event()
    , coord_(x,y)
    , viewport_(viewport)
    , buttonCode_(button)
    , action_(action)
    , mod_(mod)
{}

int MouseEvent::getEventType() {
    switch (action_) {
        case MOTION:
            // motion = move?
            return MOUSEMOVEEVENT;
            break;
        case PRESSED:
            return MOUSEPRESSEVENT;
            break;
        case RELEASED:
            return MOUSERELEASEEVENT;
            break;
        case DOUBLECLICK:
            return MOUSEDOUBLECLICKEVENT;
            break;
        case WHEEL:
            return WHEELEVENT;
            break;
        case ENTER:
            return MOUSEENTEREVENT;
            break;
        case EXIT:
            return MOUSEEXITEVENT;
            break;
        default:
            return 0;
    }
}

ivec2 MouseEvent::coord() const {
    return coord_;
}

int MouseEvent::x() const {
    return coord_.x;
}

int MouseEvent::y() const {
    return coord_.y;
}

ivec2 MouseEvent::viewport() const {
    return viewport_;
}

MouseEvent::MouseButtons MouseEvent::button() const {
    return buttonCode_;
}

MouseEvent::MouseAction MouseEvent::action() const {
    return action_;
}

Event::Modifier MouseEvent::modifiers() const {
    return mod_;
}

}
