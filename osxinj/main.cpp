#include "injector.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <dlfcn.h>
#include <stdlib.h>

#include <list>
#include <libproc.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

#include <libgen.h>

#include <CoreFoundation/CoreFoundation.h>

#define IMAGENT_FULL_NAME "IMCore.framework/imagent.app"
#define MMIMAGENT_ENABLER_DYLIB_NAME "MMIMAgentEnablerLib.dylib"

void NoteExitKQueueCallback(
                                   CFFileDescriptorRef f,
                                   CFOptionFlags       callBackTypes,
                                   void *              info
                                   )
{
    struct kevent   event;
    
//    (void) kevent( CFFileDescriptorGetNativeDescriptor(f), NULL, 0, &event, 1, NULL);
    
    
    // You've been notified!
}

static void wait_for_pid(pid_t pid)
{
    int                     kq;
    struct kevent           changes;
    
    // Create the kqueue and set it up to watch for SIGCHLD. Use the
    // new-in-10.5 EV_RECEIPT flag to ensure that we get what we expect.
    
    kq = kqueue();
    
    EV_SET(&changes, pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, NULL);
    int result = kevent(kq, &changes, 1, &changes, 1, NULL);
    
    printf("%s exited! Will try to reinject.\n", IMAGENT_FULL_NAME);
    
    return;
    
}

int main(int argc, char* argv[])
{
//    if (argc < 3)
//    {
//        fprintf(stderr, "Usage: osxinj [proc_name] [lib]\n");
//        return 0;
//    }
    
    std::list <pid_t> pids_injected;

    char path[MAXPATHLEN];
    realpath(argv[0], path);
    char *currentPath = dirname(path);
    
    sprintf(path, "%s/%s", currentPath, MMIMAGENT_ENABLER_DYLIB_NAME);

    fprintf(stderr, "%s\n", path);

    Injector inj;
    
    while (1)
    {
        
        int procCnt = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
        pid_t *pids = (pid_t *) malloc(procCnt * sizeof(pid_t));
        
        if (pids == NULL)
            return 0;
        
        memset(pids, 0, procCnt * sizeof(pid_t));
        
        proc_listpids(PROC_ALL_PIDS, 0, pids, procCnt * sizeof(pid_t));
        
        for (int i = 0; i < procCnt; i++)
        {
            if (!pids[i]) continue;
            char curPath[PROC_PIDPATHINFO_MAXSIZE] = { 0 };
            proc_pidpath(pids[i], curPath, sizeof curPath);
            size_t len = strlen(curPath);
            if (len)
            {
                if (strstr(curPath, IMAGENT_FULL_NAME))
                {
                    printf("Found %s. INJECTING PID %d.\n", curPath, pids[i]);
                    inj.inject(pids[i], path);
                    wait_for_pid(pids[i]);
                    break;
                }
            }
        }
        free (pids);
        sleep(1);
    }
    return 0;
}