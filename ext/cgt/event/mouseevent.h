/**********************************************************************
 *                                                                    *
 * cgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2011 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
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

#ifndef CGT_MOUSEEVENT_H
#define CGT_MOUSEEVENT_H

#include "cgt/event/event.h"
#include "cgt/vector.h"

namespace cgt {

class CGT_API MouseEvent : public Event {
public:
    enum MouseButtons {
        MOUSE_BUTTON_NONE =     0,
        MOUSE_BUTTON_1 =   1 << 0, MOUSE_BUTTON_LEFT   = MOUSE_BUTTON_1,
        MOUSE_BUTTON_2 =   1 << 1, MOUSE_BUTTON_MIDDLE = MOUSE_BUTTON_2,
        MOUSE_BUTTON_3 =   1 << 2, MOUSE_BUTTON_RIGHT  = MOUSE_BUTTON_3,
        MOUSE_BUTTON_4 =   1 << 3,
        MOUSE_BUTTON_5 =   1 << 4,
        MOUSE_WHEEL_DOWN = 1 << 5,
        MOUSE_WHEEL_UP =   1 << 6,
        MOUSE_WHEEL    = MOUSE_WHEEL_DOWN | MOUSE_WHEEL_UP,
        MOUSE_BUTTON_ALL = MOUSE_BUTTON_1 | MOUSE_BUTTON_2 | MOUSE_BUTTON_3 | MOUSE_BUTTON_4 | MOUSE_BUTTON_5 | MOUSE_WHEEL
    };

    enum MouseAction {
        ACTION_NONE = 0,
        MOTION      = 1 << 0,
        PRESSED     = 1 << 1,
        RELEASED    = 1 << 2, CLICK = PRESSED | RELEASED,
        DOUBLECLICK = 1 << 3,
        WHEEL       = 1 << 4,
        ENTER       = 1 << 5,
        EXIT        = 1 << 6, ENTER_EXIT = ENTER | EXIT,
        ACTION_ALL  = CLICK | MOTION | DOUBLECLICK | WHEEL | ENTER_EXIT
    };

public:
    MouseEvent(int x, int y, MouseAction action, Event::Modifier mod, MouseButtons button = MOUSE_BUTTON_NONE,
        cgt::ivec2 viewport = cgt::ivec2(-1));

    ivec2 coord() const;
    int x() const;
    int y() const;
    ivec2 viewport() const;
    MouseButtons button() const;
    MouseAction action() const;
    Event::Modifier modifiers() const;

    void setCoord(const ivec2& c) { coord_ = c; }
    void setViewport(const ivec2& viewp) { viewport_ = viewp; }

    virtual int getEventType();

protected:
    ivec2 coord_;
    ivec2 viewport_;
    MouseButtons buttonCode_;
    MouseAction action_;
    Event::Modifier mod_;
};

}

#endif //CGT_MOUSEEVENT_H
