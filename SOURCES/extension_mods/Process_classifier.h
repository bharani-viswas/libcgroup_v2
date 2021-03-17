#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>
#include "proc_probe.h"
#include "cgrules_dict.h"

#define QUEUE_NAME  "/cgroup_queue"
#define MAX_SIZE    1024
#define MSG_STOP    "exit"

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \

int cgrules_classify(int);

void* Process_clasifier()
{
    int status = cgrules_configure();
	printf("cgroup configuration : %d \n",status);
    mqd_t mq;
    char buffer[MAX_SIZE + 1];
    int must_stop = 0;
    mq = mq_open(QUEUE_NAME, O_RDONLY);
    CHECK((mqd_t)-1 != mq);

    do {
        ssize_t bytes_read;

        /* receive the message */
        bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        CHECK(bytes_read >= 0);

        buffer[bytes_read] = '\0';
        if (! strncmp(buffer, MSG_STOP, strlen(MSG_STOP)))
        {
            must_stop = 1;
        }
        else
        {
//	printf("im here");
	int pid = atoi(buffer);
	cgrules_classify(pid);
        }
	
    } while (!must_stop);

    /* cleanup */
   CHECK((mqd_t)-1 != mq_close(mq));
   CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));
   hdestroy_r(cgrules_slice_dict);
   free(cgrules_slice_dict);
   return 0;
}

int cgrules_classify(int pid)
{
ENTRY e;
ENTRY *ep;
char *procname;
int ret = cgroup_get_procname_from_procfs(pid, &procname);
if (ret == 0)
{
	e.key = procname;
        hsearch_r(e, FIND,&ep,cgrules_slice_dict);
	if(ep != NULL)
	{
		char str[20];
		sprintf(str, "%d", pid);
	        FILE *f;
        	char path[FILENAME_MAX];
		sprintf(path, "/sys/fs/cgroup/unified/%s/cgroup.procs", (char*)(ep->data));
		f = fopen(path, "w");
        	if (!f)
                	return -1;
		fputs(str,f);
    		fclose(f);
	}

}
}

