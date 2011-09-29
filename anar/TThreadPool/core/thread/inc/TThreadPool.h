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

//#include <iostream>

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
            m_stopped( false ),
            m_stopping( false )
        {
            m_mutex = new TMutex();
            m_threadNeeded = new TCondition( m_mutex );
            m_threadAvailable = new TCondition( m_mutex );

            for( size_t i = 0; i < _threadsCount; ++i )
            {
                TThread *pThread = new TThread( &TThreadPool::Execute, this );
                m_threads.push_back( pThread );
                pThread->Run();
            }
        }

        ~TThreadPool()
        {
            Stop();
            delete m_threadNeeded;
            delete m_threadAvailable;
            delete m_mutex;
            // delete threads
            threads_array_t::const_iterator iter = m_threads.begin();
            threads_array_t::const_iterator iter_end = m_threads.end();
            for( ; iter != iter_end; ++iter )
                delete( *iter );
        }

        void PushTask( typename TThreadPoolTask<_T, _P>::task_t &_task, _P _param )
        {
            TLockGuard lock( m_mutex );
            task_t *task = new task_t( _task, _param );
            m_tasks.push( task );
            m_threadNeeded->Signal();
            ++m_tasksCount;
        }

        void Stop( bool processRemainingJobs = false )
        {
            {
                // prevent more jobs from being added to the queue
                if( m_stopped )
                    return;
                
                TLockGuard lock( m_mutex );
                m_stopping = true;
            }
            if( processRemainingJobs )
            {
               // TLockGuard lock( m_mutex );
                // wait for queue to drain
                while( !m_tasks.empty() && !m_stopped )
                {
                    m_threadAvailable->Wait();
                }
            }
            // tell all threads to stop
            {
                TLockGuard lock( m_mutex );
                m_stopped = true;
            }
            m_threadNeeded->Signal();

            threads_array_t::const_iterator iter = m_threads.begin();
            threads_array_t::const_iterator iter_end = m_threads.end();
            for( ; iter != iter_end; ++iter )
                ( *iter )->Join();
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
            TThreadPool *pThis = reinterpret_cast<TThreadPool*>(_arg);
            do
            {
                task_t* task = NULL;

                {
                    // Find a job to perform
                    if( pThis->m_tasks.empty() && !pThis->m_stopped )
                    {
                        pThis->m_threadNeeded->Wait();
                    }
                    
                    TLockGuard lock( pThis->m_mutex );
                    if( !pThis->m_tasks.empty() && !pThis->m_stopped )
                    {
                        task = pThis->m_tasks.front();
                        pThis->m_tasks.pop();
                    }
                }
                
                // Execute job
                if( task )
                {
                    if( task->run() )
                    {
                        TLockGuard lock( pThis->m_mutex );
                        ++pThis->m_successfulTasks;
                    }
                    delete task;
                    task = NULL;
                }
                pThis->m_threadAvailable->Signal();
            }
            while( !pThis->m_stopped );
            
            return NULL;
        }

    private:
        taskqueue_t m_tasks;
        TMutex *m_mutex;
        TCondition *m_threadNeeded;
        TCondition *m_threadAvailable;
        threads_array_t m_threads;
        volatile bool m_stopped;
        volatile bool m_stopping;
        size_t m_successfulTasks;
        size_t m_tasksCount;
};

#endif
