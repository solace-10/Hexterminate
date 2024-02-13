#ifndef _GENESIS_TASKMANAGER_H_
#define _GENESIS_TASKMANAGER_H_

#include <list>
#include <string>

#include "timer.h"

namespace Genesis
{

// The TaskManager handles registered Tasks, which will be stepped every
// time Update() is called. The idea is that every class that needs to be
// stepped inherits from Task and implements a method in the form of
// TaskFunc. The order by which they are updated depends on their
// TaskPriority. Tasks that share the same priority are updated by the
// ordered they were registered. The TaskFunc returns a TaskStatus - the
// task will keep being updated while it returns TASK_CONTINUE. Once it
// returns TASK_STOP, the task will be removed from the manager.

class Task;
class Logger;

enum class TaskPriority
{
    System, // Event handling, input management, etc
    Physics,
    GameLogic, // All game logic related tasks
    Rendering
};

enum class TaskStatus
{
    Continue,
	Stop
};

typedef TaskStatus ( Task::*TaskFunc )( float delta );

struct TaskInfo
{
    Task* task;
    TaskFunc func;
    std::string name;
    TaskPriority priority;
    bool remove;
};

typedef std::list<TaskInfo*> TaskInfoList;

class TaskManager
{
public:
    TaskManager();
    TaskManager( Logger* pLogger );
    ~TaskManager();

    void AddTask( const std::string& name, Task* pTask, TaskFunc func, TaskPriority priority );
    void PrintTasks() const;
    void Update();
    bool IsRunning() const;
    void Stop();

private:
    void RemoveMarkedTasks();
    TaskInfoList mTasks;
    TaskInfoList mTasksToBeRemoved;
    bool mIsRunning;
    Logger* mLogger;
    Timer m_Timer;
};

inline bool TaskManager::IsRunning() const { return mIsRunning; }
inline void TaskManager::Stop() { mIsRunning = false; }

class Task
{
public:
    Task(){};
    virtual ~Task(){};
};
}

#endif