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

#ifndef CGT_EVENTLISTENER_H
#define CGT_EVENTLISTENER_H

#include "cgt/types.h"

namespace cgt {

class Event;
class KeyEvent;
class MouseEvent;
class TimeEvent;

/**
 * EventListener get called by cgt::EventHandlers to perform action with respect to the incoming events.
 * 
 * To define your own event handling, derive your class from cgt::EventListener and override the
 * event handling methods you need. You can optimize by using setEventTypes() to filter unneeded
 * method calls for events not being handled.
 */
class CGT_API EventListener {
public:
    /// Default constructor
    EventListener();

    /// Default virtual Destructor
    virtual ~EventListener();

    /**
     * Gets called on mouse pressed events.
     * \param   e   Event data
     */
    virtual void mousePressEvent(MouseEvent* e);

    /**
     * Gets called on mouse released events.
     * \param   e   Event data
     */
    virtual void mouseReleaseEvent(MouseEvent* e);

    /**
     * Gets called on mouse moved events.
     * \param   e   Event data
     */
    virtual void mouseMoveEvent(MouseEvent* e);

    /**
     * Gets called on mouse double click events.
     * \param   e   Event data
     */
    virtual void mouseDoubleClickEvent(MouseEvent* e);

    /**
     * Gets called on mouse wheel events.
     * \param   e   Event data
     */
    virtual void wheelEvent(MouseEvent* e);
    
    /**
     * Gets called on timer events.
     * \param   e   Event data
     */
    virtual void timerEvent(TimeEvent* e);

    /**
     * Gets called on key (pressed released) events.
     * \param   e   Event data
     */
    virtual void keyEvent(KeyEvent* e);

    /**
     * Gets called on every event.
     * Default implementation checks the event type and forwards the call to on of the virtual 
     * xyzEvent() methods in this class.
     * 
     * \param   e   Event data
     */
    virtual void onEvent(Event* e);

    /**
     * Sets the bit mask of events this listener is listening to.
     * Setting this to 0 will deactivate this event during broadcasting from cgt::EventHandler.
     * \note    Other event distributing systems may ignore this field.
     * \param   eventTypes  Bit mask of events this listener is listening to.
     */
    void setEventTypes(int eventTypes);

    /**
     * Gets the bit mask of events this listener is listening to.
     * \return  bitmask_
     */
    int getEventTypes() const;

private:
    /*
     * bitmask in which all eventTypes this EventListener is listening to are stored
     * could be used to deactivate an EventListener by setting eventTypes_ = 0
     */
    int eventTypes_;
};

}

#endif //CGT_EVENTLISTENER_H
