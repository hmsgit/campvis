#ifndef OBSERVER_H__
#define OBSERVER_H__

#include "tbb/include/tbb/spin_mutex.h"
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

        /**
         * Method to be called by observed objects.
         * Must be implemented in subclasses.
         */
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

    protected:
        /**
         * Notifies all registered observers.
         */
        void notifyObservers() const;

        mutable std::set<Observer*> _observers;     ///< Set of all observers for this subject
        mutable tbb::spin_mutex _localMutex;
    };

// - Generic variant ------------------------------------------------------------------------------

    /**
     * Templated observer implementation receiving an object of type \a T during notification.
     * \sa GenericObservable, GenericObserverArgs
     * \tparam  T   Type of object to pass during notification
     */
    template<typename T>
    class GenericObserver {
    public:
        GenericObserver() {};
        virtual ~GenericObserver() {};

        /**
         * Method to be called by observed objects, must be implemented in subclasses.
         * \sa GenericObserverArgs
         * \param args  Argument struct beiug passed by observable.
         */
        virtual void onNotify(const T& args) = 0;
    };

    /**
     * Templated observable implementation passing an object of type T during notification.
     * \sa GenericObserver, GenericObserverArgs
     * \tparam  T   Type of object to pass during notification
     */
    template<typename T>
    class GenericObservable {
    public:
        GenericObservable() {};
        virtual ~GenericObservable() {};

        /**
         * Adds the given GenericObserver<T> to the observer set.
         * \a o will be notified during Observable::notifyObservers().
         * \param o     GenericObserver<T> to add
         */
        void addObserver(GenericObserver<T>* o) const {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _observers.insert(o);
        };

        /**
         * Removes the given GenericObserver<T> from the observer set.
         * \a o will no longer be notified during Observable::notifyObservers().
         * \param o     GenericObserver<T> to remove
         */
        void removeObserver(GenericObserver<T>* o) const {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _observers.erase(o);
        };

    protected:
        /**
         * Notifies all registered observers.
         * \sa GenericObserverArgs
         * \param args  Argument struct to be passed to observer.
         */
        void notifyObservers(const T& args) const {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            for (std::set< GenericObserver<T>* >::iterator it = _observers.begin(); it != _observers.end(); ++it) {
                (*it)->onNotify(args);
            }
        };

        mutable std::set< GenericObserver<T>* > _observers;     ///< Set of all observers for this subject
        mutable tbb::spin_mutex _localMutex;
    };


    template<typename T>
    struct GenericObserverArgs {
        /**
         * Creates a new GenericObserverArgs.
         * \param subject Subject that emits the notification
         */
        GenericObserverArgs(const T* subject)
            : _subject(subject)
        {}

        const T* _subject;          ///< Subject that emits the notification
    };

}

#endif // OBSERVER_H__
