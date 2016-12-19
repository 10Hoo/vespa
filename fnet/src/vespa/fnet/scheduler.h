// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/fastos/time.h>

/**
 * An object of this class handles scheduling of @ref FNET_Task
 * objects. A task may be scheduled to be performed in a given number
 * of seconds. A scheduled task may also be unscheduled to cancel the
 * performing of that task. A scheduler object does not have its own
 * thread, but depends on being invoked regularely to perform pending
 * tasks.
 **/
class FNET_Scheduler
{
public:

    enum scheduler_constants {
        SLOT_TICK   =   10,
        NUM_SLOTS   = 4096,
        SLOTS_MASK  = 4095,
        SLOTS_SHIFT =   12
    };

private:
    FNET_Cond    _cond;
    FNET_Task   *_slots[NUM_SLOTS + 1];
    FastOS_Time  _next;
    FastOS_Time  _now;
    FastOS_Time *_sampler;
    uint32_t     _currIter;
    uint32_t     _currSlot;
    FNET_Task   *_currPt;
    FNET_Task   *_tailPt;
    FNET_Task   *_performing;
    bool         _waitTask;

    FNET_Scheduler(const FNET_Scheduler &);
    FNET_Scheduler &operator=(const FNET_Scheduler &);

    void Lock()      { _cond.Lock();      }
    void Unlock()    { _cond.Unlock();    }
    void Wait()      { _cond.Wait();      }
    void Broadcast() { _cond.Broadcast(); }


    FNET_Task *GetTask()
    {
        return _currPt;
    }


    void FirstTask(uint32_t slot)
    {
        _currPt = _slots[slot];
        _tailPt = (_currPt != NULL) ?
                  _currPt->_task_prev : NULL;
    }


    void NextTask()
    {
        _currPt = (_currPt != _tailPt) ?
                  _currPt->_task_next : NULL;
    }


    void AdjustCurrPt()
    {
        _currPt = (_currPt != _tailPt) ?
                  _currPt->_task_next : NULL;
    }


    void AdjustTailPt()
    {
        _tailPt = _tailPt->_task_prev;
    }


    void LinkIn(FNET_Task *task)
    {
        FNET_Task **head = &(_slots[task->_task_slot]);

        if ((*head) == NULL) {
            (*head) = task;
            task->_task_next = task;
            task->_task_prev = task;
        } else {
            task->_task_next = (*head);
            task->_task_prev = (*head)->_task_prev;
            (*head)->_task_prev->_task_next = task;
            (*head)->_task_prev = task;
        }
    }


    void LinkOut(FNET_Task *task)
    {
        FNET_Task **head = &(_slots[task->_task_slot]);

        if (task == _currPt)
            AdjustCurrPt();
        else if (task == _tailPt)
            AdjustTailPt();

        if (task->_task_next == task) {
            (*head) = NULL;
        } else {
            task->_task_prev->_task_next = task->_task_next;
            task->_task_next->_task_prev = task->_task_prev;
            if ((*head) == task)
                (*head) = task->_task_next;
        }
        task->_task_next = NULL;
        task->_task_prev = NULL;
    }


    static bool IsActive(FNET_Task *task)
    { return task->_task_next != NULL; }


    bool IsPerforming(FNET_Task *task)
    { return task == _performing; }


    void BeforeTask(FNET_Task *task)
    {
        _performing = task;
        Unlock();
    }


    void AfterTask()
    {
        Lock();
        _performing = NULL;
        if (_waitTask) {
            _waitTask = false;
            Broadcast();
        }
    }


    void WaitTask(FNET_Task *task)
    {
        while (IsPerforming(task)) {
            _waitTask = true;
            Wait();
        }
    }


    void PerformTasks(uint32_t slot, uint32_t iter)
    {
        FirstTask(slot);
        for (FNET_Task *task; (task = GetTask()) != NULL; ) {
            NextTask();

            if (task->_task_iter == iter) {
                LinkOut(task);
                BeforeTask(task);
                task->PerformTask(); // PERFORM TASK
                AfterTask();
            }
        }
    }

public:

    /**
     * Construct a scheduler.
     *
     * @param sampler if given, this object will be used to obtain the
     *                time when the @ref CheckTasks method is invoked. If a
     *                sampler is not given, time sampling will be
     *                handled internally.
     * @param now if given, indicates the current time. This value is
     *            used by the constructor to init internal variables.
     **/
    FNET_Scheduler(FastOS_Time *sampler = NULL,
                   FastOS_Time *now = NULL);
    virtual ~FNET_Scheduler();


    /**
     * Schedule a task to be performed in the given amount of
     * seconds.
     *
     * @param task the task to be scheduled.
     * @param seconds the number of seconds until the task
     *                should be performed.
     **/
    void Schedule(FNET_Task *task, double seconds);


    /**
     * Schedule a task to be performed as soon as possible.
     *
     * @param task the task to be scheduled.
     **/
    void ScheduleNow(FNET_Task *task);


    /**
     * Unschedule the given task. If the task is currently being
     * performed, this method will block until the task is
     * completed. This means that a task trying to unschedule itself
     * will result in a deadlock.
     *
     * @param task the task to unschedule.
     **/
    void Unschedule(FNET_Task *task);


    /**
     * This method does the same as the @ref Unschedule method, but also
     * makes sure that the task may not be scheduled in the future.
     **/
    void Kill(FNET_Task *task);


    /**
     * Print all currently scheduled tasks to the given file stream
     * (default is stdout). This method may be used for debugging.
     *
     * @param dst where to print the contents of this scheduler
     **/
    void Print(FILE *dst = stdout);


    /**
     * Obtain a pointer to the current time sampler used by this
     * scheduler. The returned object may only be used in the thread
     * servicing this scheduler; this includes all tasks performed by
     * this scheduler.
     *
     * @return pointer to current time sampler.
     **/
    FastOS_Time *GetTimeSampler() { return _sampler; }


    /**
     * Perform pending tasks. This method should be invoked regularly.
     **/
    void CheckTasks();
};

