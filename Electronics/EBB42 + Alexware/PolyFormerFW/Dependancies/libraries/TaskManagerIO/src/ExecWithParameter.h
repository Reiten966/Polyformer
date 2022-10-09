/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANAGERIO_EXECWITHPARAMETER_H
#define TASKMANAGERIO_EXECWITHPARAMETER_H

/**
 * @file ExecWithParameter.h
 *
 * Provides convenience helper classes that allow for function callbacks with parameters
 */

#include "TaskTypes.h"

/**
 * Gives the ability to register a task with task manager that will call a function with a stored parameter
 * An example usage would be to store a Stream upon which to write to. Note that if you use the `new` method below
 * you absolutely must provide the deleteWhenDone defaulted parameter as true.
 *
 * taskManager.scheduleFixedRate(1000, new ExecWithParameter<Stream*>(&Serial), TIME_MILLIS, true);
 *
 * To use static memory, create an ExecWithParameter instance **globally**, and then pass a pointer to the schedule.
 *
 * @tparam TParam The type that you want to store until called back.
 */
template <class TParam> class ExecWithParameter : public Executable {
private:
    void (*fnParam)(TParam);
    TParam param;
public:
    /**
     * Constructs an instance giving the function to call and the parameter to pass to the function
     * @param fn the function to be called.
     * @param storedParam the parameter to pass to the function
     */
    ExecWithParameter(void (*fn)(TParam), TParam storedParam) : fnParam(fn), param(storedParam){}
    ~ExecWithParameter() override = default;

    void exec() override {
        fnParam(param);
    }
};

/**
 * Similar to ExecWithParameter class, but this allows for two parameters.
 * @see ExecWithParameter
 *
 * @tparam TParam1 The 1st type that you want to store until called back.
 * @tparam TParam2 The 2nd type that you want to store until called back.
 */
template <class TParam1, class TParam2> class ExecWith2Parameters : public Executable {
private:
    void (*fnParam)(TParam1, TParam2);
    TParam1 param1;
    TParam2 param2;
public:
    /**
     * Constructs an instance giving the function to call and the two parameters to pass to the function
     * @param fn the function to call, that must be compatible with the two parameter types
     * @param storedParam1 the first parameter to pass to the function
     * @param storedParam2 the second parameter to pass to the function
     */
    ExecWith2Parameters(void (*fn)(TParam1, TParam2), TParam1 storedParam1, TParam2 storedParam2 ) :
                        fnParam(fn), param1(storedParam1), param2(storedParam2) {}
    ~ExecWith2Parameters() override = default;

    void exec() override {
        fnParam(param1, param2);
    }
};


#endif //TASKMANAGERIO_EXECWITHPARAMETER_H
