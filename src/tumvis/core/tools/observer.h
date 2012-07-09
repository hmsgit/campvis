#ifndef OBSERVER_H__
#define OBSERVER_H__

#include <set>

namespace TUMVis {

    /**
     * Minimal implementation of the observer pattern.
     * \sa Observable
     * \todo    Add more advanced version supporting arguments during notification.
     */
    class Observer {
    public:
        Observer();
        virtual ~Observer();

        virtual void onNotify() = 0;
    };


    /**
     * Minimal implementation of the observer pattern.
     * \sa Observer
     */
    class Observable {
    public:
        Observable();
        virtual ~Observable();

        /**
         * Adds the given Observer to the observer set.
         * \a o will be notified during Observable::notifyObservers().
         * \param o     Observer to add
         */
        void addObserver(Observer* o) const;

        /**
         * Removes the given Observer from the observer set.
         * \a o will no longer be notified during Observable::notifyObservers().
         * \param o     Observer to remove
         */
        void removeObserver(Observer* o) const;

    private:
        /**
         * Notifies all registered observers.
         */
        void notifyObservers() const;

        mutable std::set<Observer*> _observers;     ///< Set of all observers for this subject
    };

}

#endif // OBSERVER_H__
