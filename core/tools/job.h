// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef JOB_H__
#define JOB_H__

#include "tgt/assert.h"

#include "core/coreapi.h"

namespace campvis {

    /**
     * Abstract base class for a job that can be executed.
     */
    class CAMPVIS_CORE_API AbstractJob {
    public:
        /**
         * Pure virtual destructor.
         */
        virtual ~AbstractJob() {};

        /**
         * Executes the job, to be overwritten by subclasses.
         */
        virtual void execute() = 0;

        void operator()() {
            this->execute();
        }
    };

// = Specific Jobs ================================================================================

    /**
     * Specific job, that is calling a member function pasing no argument.
     */
    template<class T>
    class CallMemberFuncJob : public AbstractJob {
    public:
        /**
         * Creates an new job, that is calling \a callee on \a target pasing no argument.
         * \param   target  Target object
         * \param   callee  Member function to call
         */
        CallMemberFuncJob(T* target, void (T::*callee)())
            : _target(target)
            , _callee(callee)
        {
            tgtAssert(_target != 0, "Target object must not be 0.");
            tgtAssert(_callee != 0, "Target member function pointer must not be 0.");
        }

        /**
         * Destructor, nothing to do here
         */
        ~CallMemberFuncJob() {};

        /**
         * Executes this job by calling the member function.
         */
        virtual void execute() {
            (_target->*_callee)();
        }

    protected:
        T* _target;                 ///< Target object
        void (T::*_callee)();       ///< Member function to call
    };

    /**
     * Specific job, that is calling a member function pasing a single argument.
     */
    template<class T, class A1>
    class CallMemberFunc1ArgJob : public AbstractJob {
    public:
        /**
         * Creates an new job, that is calling \a callee on \a target pasing \a arg1 as single argument.
         * \param   target  Target object
         * \param   callee  Member function to call
         * \param   arg1    Argument to pass to \a callee
         */
        CallMemberFunc1ArgJob(T* target, void (T::*callee)(A1), A1 arg1)
            : _target(target)
            , _callee(callee)
            , _arg1(arg1)
        {
            tgtAssert(_target != 0, "Target object must not be 0.");
            tgtAssert(_callee != 0, "Target member function pointer must not be 0.");
        }

        /**
         * Destructor, nothing to do here
         */
        ~CallMemberFunc1ArgJob() {};

        /**
         * Executes this job by calling the member function.
         */
        virtual void execute() {
            (_target->*_callee)(_arg1);
        }

    protected:
        T* _target;                 ///< Target object
        void (T::*_callee)(A1);     ///< Member function to call
        A1 _arg1;                   ///< Argument to pass to \a callee
    };
    
    /**
     * Specific job, that is calling a member function pasing a single argument.
     */
    template<class A1>
    class CallFunc1ArgJob : public AbstractJob {
    public:
        /**
         * Creates an new job, that is calling \a callee on \a target pasing \a arg1 as single argument.
         * \param   target  Target object
         * \param   callee  Member function to call
         * \param   arg1    Argument to pass to \a callee
         */
        CallFunc1ArgJob(void (*callee)(A1), A1 arg1)
            : _callee(callee)
            , _arg1(arg1)
        {
            tgtAssert(_callee != 0, "Target member function pointer must not be 0.");
        }

        /**
         * Destructor, nothing to do here
         */
        ~CallFunc1ArgJob() {};

        /**
         * Executes this job by calling the member function.
         */
        virtual void execute() {
            (*_callee)(_arg1);
        }

    protected:
        void (*_callee)(A1);        ///< Function to call
        A1 _arg1;                   ///< Argument to pass to \a callee
    };

    /**
     * Specific job, that is calling a member function pasing a single argument.
     */
    template<class T, class A1, class A2>
    class CallMemberFunc2ArgJob : public AbstractJob {
    public:
        /**
         * Creates an new job, that is calling \a callee on \a target pasing \a arg1 and \a arg2 as arguments.
         * \param   target  Target object
         * \param   callee  Member function to call
         * \param   arg1    First argument to pass to \a callee
         * \param   arg2    Second argument to pass to \a callee
         */
        CallMemberFunc2ArgJob(T* target, void (T::*callee)(A1, A2), A1 arg1, A2 arg2)
            : _target(target)
            , _callee(callee)
            , _arg1(arg1)
            , _arg2(arg2)
        {
            tgtAssert(_target != 0, "Target object must not be 0.");
            tgtAssert(_callee != 0, "Target member function pointer must not be 0.");
        }

        /**
         * Destructor, nothing to do here
         */
        ~CallMemberFunc2ArgJob() {};

        /**
         * Executes this job by calling the member function.
         */
        virtual void execute() {
            (_target->*_callee)(_arg1, _arg2);
        }

    protected:
        T* _target;                 ///< Target object
        void (T::*_callee)(A1, A2); ///< Member function to call
        A1 _arg1;                   ///< First argument to pass to \a callee
        A2 _arg2;                   ///< Second argument to pass to \a callee
    };
    
    /**
     * Specific job, that is calling a function passing two arguments.
     */
    template<class A1, class A2>
    class CallFunc2ArgJob : public AbstractJob {
    public:
        /**
         * Creates an new job, that is calling \a callee passing \a arg1 and \a arg2 as arguments.
         * \param   callee  Function to call
         * \param   arg1    First argument to pass to \a callee
         * \param   arg2    Second argument to pass to \a callee
         */
        CallFunc2ArgJob(void (*callee)(A1, A2), A1 arg1, A2 arg2)
            : _callee(callee)
            , _arg1(arg1)
            , _arg2(arg2)
        {
            tgtAssert(_callee != 0, "Target member function pointer must not be 0.");
        }

        /**
         * Destructor, nothing to do here
         */
        ~CallFunc2ArgJob() {};

        /**
         * Executes this job by calling the function.
         */
        virtual void execute() {
            (*_callee)(_arg1, _arg2);
        }

    protected:
        void (*_callee)(A1, A2);    ///< Function to call
        A1 _arg1;                   ///< First Argument to pass to \a callee
        A2 _arg2;                   ///< Second Argument to pass to \a callee
    };

// = Helper functions for easier creation of jobs =================================================

    /**
     * Creates a new CallMemberFuncJob on the heap for the object \a target.
     * \note    The caller takes ownership of the returned pointer.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \return  Pointer to the newly created CallMemberFuncJob. Caller has ownership!
     */
    template<class T>
    CallMemberFuncJob<T>* makeJobOnHeap(T* target, void (T::*callee)()) {
        return new CallMemberFuncJob<T>(target, callee);
    }

    /**
     * Creates a new CallMemberFuncJob on the stack for the object \a target.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \return  The newly created CallMemberFuncJob
     */
    template<class T>
    CallMemberFuncJob<T> makeJob(T* target, void (T::*callee)()) {
        return CallMemberFuncJob<T>(target, callee);
    }

    /**
     * Creates a new CallMemberFunc1ArgJob on the heap  for the object \a target.
     * \note    The caller takes ownership of the returned pointer.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg1    First argument to pass to \callee.
     * \return  Pointer to the newly created CallMemberFunc1ArgJob. Caller has ownership!
     */
    template<class T, class A1>
    CallMemberFunc1ArgJob<T, A1>* makeJobOnHeap(T* target, void (T::*callee)(A1), A1 arg1) {
        return new CallMemberFunc1ArgJob<T, A1>(target, callee, arg1);
    }

    /**
     * Creates a new CallMemberFunc1ArgJob on the stack  for the object \a target.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg1    First argument to pass to \callee.
     * \return  The newly created CallMemberFunc1ArgJob.
     */
    template<class T, class A1>
    CallMemberFunc1ArgJob<T, A1> makeJob(T* target, void (T::*callee)(A1), A1 arg1) {
        return CallMemberFunc1ArgJob<T, A1>(target, callee, arg1);
    }

    /**
     * Creates a new CallFunc1ArgJob on the heap  for the object \a target.
     * \note    The caller takes ownership of the returned pointer.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg1    First argument to pass to \callee.
     * \return  Pointer to the newly created CallFunc1ArgJob. Caller has ownership!
     */
    template<class A1>
    CallFunc1ArgJob<A1>* makeJobOnHeap(void (*callee)(A1), A1 arg1) {
        return new CallFunc1ArgJob<A1>(callee, arg1);
    }

    /**
     * Creates a new CallFunc1ArgJob on the stack  for the object \a target.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg1    First argument to pass to \callee.
     * \return  The newly created CallFunc1ArgJob.
     */
    template<class A1>
    CallFunc1ArgJob<A1> makeJob(void (*callee)(A1), A1 arg1) {
        return CallFunc1ArgJob<A1>(callee, arg1);
    }

    /**
     * Creates a new CallMemberFunc1ArgJob on the heap  for the object \a target.
     * \note    The caller takes ownership of the returned pointer.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg1    First argument to pass to \callee.
     * \return  Pointer to the newly created CallMemberFunc1ArgJob. Caller has ownership!
     */
    template<class T, class A1, class A2>
    CallMemberFunc2ArgJob<T, A1, A2>* makeJobOnHeap(T* target, void (T::*callee)(A1, A2), A1 arg1, A2 arg2) {
        return new CallMemberFunc2ArgJob<T, A1, A2>(target, callee, arg1, arg2);
    }

    /**
     * Creates a new CallMemberFunc1ArgJob on the stack  for the object \a target.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg1    First argument to pass to \callee.
     * \return  The newly created CallMemberFunc1ArgJob.
     */
    template<class T, class A1, class A2>
    CallMemberFunc2ArgJob<T, A1, A2> makeJob(T* target, void (T::*callee)(A1, A2), A1 arg1, A2 arg2) {
        return CallMemberFunc2ArgJob<T, A1, A2>(target, callee, arg1, arg2);
    }

    /**
     * Creates a new CallFunc2ArgJob on the heap  for the object \a target.
     * \note    The caller takes ownership of the returned pointer.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg1    First argument to pass to \callee.
     * \param   arg2    Second argument to pass to \callee.
     * \return  Pointer to the newly created CallFunc2ArgJob. Caller has ownership!
     */
    template<class A1, class A2>
    CallFunc2ArgJob<A1, A2>* makeJobOnHeap(void (*callee)(A1, A2), A1 arg1, A2 arg2) {
        return new CallFunc2ArgJob<A1, A2>(callee, arg1, arg2);
    }

    /**
     * Creates a new CallFunc2ArgJob on the stack  for the object \a target.
     * \param   target  Target object to call method from.
     * \param   callee  Pointer to method to call.
     * \param   arg2    Second argument to pass to \callee.
     * \return  The newly created CallFunc2ArgJob.
     */
    template<class A1, class A2>
    CallFunc2ArgJob<A1, A2> makeJob(void (*callee)(A1, A2), A1 arg1, A2 arg2) {
        return CallFunc2ArgJob<A1, A2>(callee, arg1, arg2);
    }
}

#endif // JOB_H__