/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2014               *
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

#include "cgt/event/eventlistener.h"

#include "cgt/event/event.h"
#include "cgt/event/keyevent.h"
#include "cgt/event/mouseevent.h"
#include "cgt/event/timeevent.h"
#include <typeinfo>

namespace cgt {

EventListener::EventListener() {
    eventTypes_ = Event::ALLEVENTTYPES;
}

EventListener::~EventListener() {
}

void EventListener::onEvent(Event* e) {
    /*
        since we already checked which type we have we can safely use
        the faster static_cast (rl)
    */
    if (typeid(*e) == typeid(MouseEvent)) {
        MouseEvent* me = static_cast<MouseEvent*>(e);
        if (me->action() == MouseEvent::PRESSED)
            mousePressEvent(me);
        else if (me->action() == MouseEvent::RELEASED)
            mouseReleaseEvent(me);
        else if (me->action() == MouseEvent::MOTION)
            mouseMoveEvent(me);
        else if (me->action() == MouseEvent::DOUBLECLICK)
            mouseDoubleClickEvent(me);
        else if (me->action() == MouseEvent::WHEEL)
            wheelEvent(me);
    }
    else if (typeid(*e) == typeid(KeyEvent)) {
        keyEvent(static_cast<KeyEvent*>(e));
    }
    else if (typeid(*e) == typeid(TimeEvent)) {
        timerEvent(static_cast<TimeEvent*>(e));
    }
}

void EventListener::setEventTypes(int eventTypes){
    eventTypes_ = eventTypes;
}

int EventListener::getEventTypes() const{
    return eventTypes_;
}

void EventListener::mousePressEvent(MouseEvent* e) {
    e->ignore();
}

void EventListener::mouseReleaseEvent(MouseEvent* e) {
    e->ignore();
}

void EventListener::mouseMoveEvent(MouseEvent* e) {
    e->ignore();
}

void EventListener::mouseDoubleClickEvent(MouseEvent* e) {
    e->ignore();
}

void EventListener::wheelEvent(MouseEvent* e) {
    e->ignore();
}

void EventListener::timerEvent(TimeEvent* e) {
    e->ignore();
}

void EventListener::keyEvent(KeyEvent* e) {
    e->ignore();
}

} // namespace cgt
