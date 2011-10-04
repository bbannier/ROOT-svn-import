// @(#)root/thread:$Id$
// Author: Anar Manafov   20/09/2011

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TThreadPool
#define ROOT_TThreadPool


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TThreadPool                                                          //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TMutex
#include "TMutex.h"
#endif
#ifndef ROOT_TCondition
#include "TCondition.h"
#endif
// STD
#include <queue>
#include <vector>
#include <iostream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TNonCopyable                                                         //
// Class which makes child to be non-copyable object.                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
class TNonCopyable
{
    protected:
        TNonCopyable()
        {}
        ~TNonCopyable()
        {}
    private:
        TNonCopyable( const TNonCopyable& );
        const TNonCopyable& operator=( const TNonCopyable& );
};
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TThreadPoolTaskImp                                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
template <class _T, class _P>
class TThreadPoolTaskImp
{
    public:
        bool run( _P &_param )
        {
            _T *pThis = reinterpret_cast<_T *>( this );
            return pThis->runTask( _param );
        }
};
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TThreadPoolTask                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
template <class _T, class _P>
class TThreadPoolTask
{
    public:
        typedef TThreadPoolTaskImp<_T, _P> task_t;

    public:
        TThreadPoolTask( task_t &_task, _P &_param ):
            m_task( _task ),
            m_taskParam( _param )
        {
        }
        bool run()
        {
            return m_task.run( m_taskParam );
        }

    private:
        task_t &m_task;
        _P m_taskParam;
};
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TThreadPool                                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
template <class _T, class _P>
class TThreadPool : public TNonCopyable
{
        typedef TThreadPoolTask<_T, _P> task_t;
        typedef std::queue<task_t*> taskqueue_t;
        typedef std::vector<TThread*> threads_array_t;

    public:
        TThreadPool( size_t _threadsCount ):
            m_stopped( false )
        {
            m_threadNeeded = new TCondition( &m_mutex );
            m_threadAvailable = new TCondition( &m_mutex );

            m_threadFinished = new TCondition( &m_mutex );

            for( size_t i = 0; i < _threadsCount; ++i )
            {
                TThread *pThread = new TThread( &TThreadPool::Execute, this );
                m_threads.push_back( pThread );
                pThread->Run();
            }

            m_threadJoinHelper =  new TThread( &TThreadPool::JoinHelper, this );
            //m_threadJoinHelper->Run();
        }

        ~TThreadPool()
        {
            Stop();
            // deleting threads
            threads_array_t::const_iterator iter = m_threads.begin();
            threads_array_t::const_iterator iter_end = m_threads.end();
            for( ; iter != iter_end; ++iter )
                delete( *iter );

            delete m_threadJoinHelper;

            delete m_threadNeeded;
            delete m_threadAvailable;
        }

        void PushTask( typename TThreadPoolTask<_T, _P>::task_t &_task, _P _param )
        {
            {
                DbgLog( "Main thread. Try to push a task" );

                TLockGuard lock( &m_mutex );
                task_t *task = new task_t( _task, _param );
                m_tasks.push( task );
                ++m_tasksCount;

                DbgLog( "Main thread. the task is pushed" );
            }
            m_threadNeeded->Broadcast();

        }

        void Stop( bool processRemainingJobs = false )
        {
            // prevent more jobs from being added to the queue
            if( m_stopped )
                return;

            if( processRemainingJobs )
            {
                TLockGuard lock( &m_mutex );
                // wait for queue to drain
                while( !m_tasks.empty() && !m_stopped )
                {
                    DbgLog( "Main thread is waiting" );
                    m_threadAvailable->Wait();
                    DbgLog( "Main thread is DONE waiting" );
                }
            }
            // tell all threads to stop
            {
                TLockGuard lock( &m_mutex );
                m_stopped = true;
                DbgLog( "Main threads requests to STOP" );
            }
            m_threadNeeded->Broadcast();

            // Waiting for all threads to complete
            m_threadJoinHelper->Run();
            m_threadJoinHelper->Join();
        }

        size_t TasksCount() const
        {
            return m_tasksCount;
        }

        size_t SuccessfulTasks() const
        {
            return m_successfulTasks;
        }

    private:
        static void* Execute( void *_arg )
        {
            TThreadPool *pThis = reinterpret_cast<TThreadPool*>( _arg );

            while( !pThis->m_stopped )
            {
                task_t *task( NULL );

                std::stringstream ss;
                ss
                        << "(" << TThread::SelfId() << ") check for a task: " << pThis->m_stopped
                        << " Number of Tasks: " << pThis->m_tasks.size();
                pThis->DbgLog( ss.str() );

                // There is a task, let's take it
                {
                    TLockGuard lock( &pThis->m_mutex );
                    // Find a task to perform
                    if( pThis->m_tasks.empty() && !pThis->m_stopped )
                    {
                        ss.str( "" );
                        ss << "(" << TThread::SelfId() << ") waiting for a task: " << pThis->m_stopped;
                        pThis->DbgLog( ss.str() );

                        // No tasks, we wait for a task to come
                        pThis->m_threadNeeded->Wait();

                        ss.str( "" );
                        ss << "(" << TThread::SelfId() << ") done waiting for a task: " << pThis->m_stopped;
                        pThis->DbgLog( ss.str() );
                    }

                    {
                        if( !pThis->m_tasks.empty() )
                        {
                            task = pThis->m_tasks.front();
                            pThis->m_tasks.pop();

                            ss.str( "" );
                            ss << "(" << TThread::SelfId() << ") getting a task: " << pThis->m_stopped;
                            pThis->DbgLog( ss.str() );
                        }
                        ss.str( "" );
                        ss << "(" << TThread::SelfId() << ") done check for a task: " << pThis->m_stopped;
                        pThis->DbgLog( ss.str() );
                    }
                }

                // Execute the task
                if( task )
                {
                    ss.str( "" );
                    ss << "(" << TThread::SelfId() << ") run a task: " << pThis->m_stopped;
                    pThis->DbgLog( ss.str() );

                    if( task->run() )
                    {
                        TLockGuard lock( &pThis->m_mutex );
                        ++pThis->m_successfulTasks;
                    }
                    delete task;
                    task = NULL;

                    ss.str( "" );
                    ss << "(" << TThread::SelfId() << ") done running a task: " << pThis->m_stopped;
                    pThis->DbgLog( ss.str() );
                }
                // Task is done, report that the thread is free
                pThis->m_threadAvailable->Broadcast();
            }

            std::stringstream ss;
            ss << "(" << TThread::SelfId() << ") **** DONE *** ";
            pThis->DbgLog( ss.str() );
            return NULL;
        }

        static void *JoinHelper( void *_arg )
        {
            TThreadPool *pThis = reinterpret_cast<TThreadPool*>( _arg );

            bool bAllDone( false );
            while( !bAllDone )
            {
                pThis->m_threadNeeded->Broadcast();

                TLockGuard lock( &pThis->m_mutex );
                pThis->m_threadAvailable->TimedWaitRelative( 100 );
                threads_array_t::const_iterator iter = pThis->m_threads.begin();
                threads_array_t::const_iterator iter_end = pThis->m_threads.end();
                bAllDone = true;
                for( ; iter != iter_end; ++iter )
                {
                    if(( *iter )->GetState() == TThread::kRunningState )
                    {
                        bAllDone = false;
                        break;
                    }
                }
            }
            return NULL;
        }

        void DbgLog( const std::string &_msg )
        {
            TLockGuard lock( &m_dbgOutputMutex );
            std::cout << _msg << std::endl;
        }

    private:
        taskqueue_t m_tasks;
        TMutex m_mutex;
        TCondition *m_threadNeeded;
        TCondition *m_threadAvailable;
        TCondition *m_threadFinished;
        threads_array_t m_threads;
        TThread *m_threadJoinHelper;
        volatile bool m_stopped;
        size_t m_successfulTasks;
        size_t m_tasksCount;
        TMutex m_dbgOutputMutex;
};

#endif
