// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#include <stdlib.h>

#include <vespa/fastos/fastos.h>
#include "tests.h"
#include "jobs.h"
#include "base_thread.hpp"

class ThreadTest : public BaseForThreadTest
{
   int Main ();

   void SingleThreadJoinWaitMultipleTest(int variant)
   {
      bool rc=false;

      char testName[300];

      sprintf(testName, "Single Thread Join Wait Multiple Test %d", variant);
      TestHeader(testName);

      FastOS_ThreadPool pool(128*1024);

      const int testThreads=5;
      int lastThreadNum = testThreads-1;
      int i;

      Job jobs[testThreads];

      FastOS_Mutex jobMutex;

      // The mutex is used to pause the first threads until we have created
      // the last one.
      jobMutex.Lock();

      for(i=0; i<lastThreadNum; i++)
      {
         jobs[i].code = WAIT_FOR_THREAD_TO_FINISH;
         jobs[i].mutex = &jobMutex;
         jobs[i].ownThread = pool.NewThread(this,
                 static_cast<void *>(&jobs[i]));

         rc = (jobs[i].ownThread != NULL);
         Progress(rc, "Creating Thread %d", i+1);

         if(!rc)
            break;
      }

      if(rc)
      {
         jobs[lastThreadNum].code = (((variant & 2) != 0) ? NOP : PRINT_MESSAGE_AND_WAIT3SEC);
         jobs[lastThreadNum].message = strdup("This is the thread that others wait for.");

         FastOS_ThreadInterface *lastThread;

         lastThread = pool.NewThread(this,
                                     static_cast<void *>
                                     (&jobs[lastThreadNum]));

         rc = (lastThread != NULL);
         Progress(rc, "Creating last thread");

         if(rc)
         {
            for(i=0; i<lastThreadNum; i++)
            {
               jobs[i].otherThread = lastThread;
            }
         }
      }

      jobMutex.Unlock();

      if((variant & 1) != 0)
      {
         for(i=0; i<lastThreadNum; i++)
         {
            Progress(true, "Waiting for thread %d to finish using Join()", i+1);
            jobs[i].ownThread->Join();
            Progress(true, "Thread %d finished.", i+1);
         }
      }

      Progress(true, "Closing pool.");
      pool.Close();
      Progress(true, "Pool closed.");
      PrintSeparator();
   }

};

int ThreadTest::Main ()
{
   printf("grep for the string '%s' to detect failures.\n\n", failString);
   time_t before = time(0);

   SingleThreadJoinWaitMultipleTest(0);
   { time_t now = time(0); printf("[%ld seconds]\n", now-before); before = now; }
   SingleThreadJoinWaitMultipleTest(1);
   { time_t now = time(0); printf("[%ld seconds]\n", now-before); before = now; }
   SingleThreadJoinWaitMultipleTest(2);
   { time_t now = time(0); printf("[%ld seconds]\n", now-before); before = now; }
   SingleThreadJoinWaitMultipleTest(3);
   { time_t now = time(0); printf("[%ld seconds]\n", now-before); before = now; }
   SingleThreadJoinWaitMultipleTest(2);
   { time_t now = time(0); printf("[%ld seconds]\n", now-before); before = now; }
   SingleThreadJoinWaitMultipleTest(1);
   { time_t now = time(0); printf("[%ld seconds]\n", now-before); before = now; }
   SingleThreadJoinWaitMultipleTest(0);
   { time_t now = time(0); printf("[%ld seconds]\n", now-before); before = now; }

   printf("END OF TEST (%s)\n", _argv[0]);

   return 0;
}

int main (int argc, char **argv)
{
   ThreadTest app;
   setvbuf(stdout, NULL, _IOLBF, 8192);
   return app.Entry(argc, argv);
}
