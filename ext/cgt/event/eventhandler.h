/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2011 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
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

#ifndef TGT_EVENTHANDLER_H
#define TGT_EVENTHANDLER_H

#include <deque>

#include "cgt/event/eventlistener.h"
#include "cgt/event/event.h"

namespace tgt {

class TGT_API EventHandler {
protected:
    std::deque<EventListener*> listeners_; ///< The deque containing all the listeners.

public:
    EventHandler();
    virtual ~EventHandler() {}

    /**
     * Add a listener to the \a listeners_ deques back.
     */
    void addEventListenerToBack(EventListener* e);

    /**
     * Add a listener to the \a listeners_ deques front.
     */
    void addEventListenerToFront(EventListener* e);

    /**
     * Remove a listener from the \a listeners_ deque.
     */
    void removeEventListener(EventListener* e);

    /**
     * Removes all listeners from \a listeners_ deque.
     */
    void clearEventListeners();

    /**
     * Send an event to all listeners in the \a listeners_ deque.  If a listener accepts the event,
     * the broadcastEvent is aborted.  All broadcasted events are deleted at the end of this method.
     */
    virtual void broadcastEvent(Event* e);

    /**
     * Returns the number of listeners associated.
     */
    size_t getNumEventListeners() const;
};

}

#endif //TGT_EVENTHANDLER_H
