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

int cgrules_search_classify(int);

void* Process_clasifier()
{
	int status = create_slice_map();
	printf("slice map status : %d \n",status);
        if(status == -1)
        {
                exit(0);
        }
        status = cgrules_configure();
	printf("cgrules status : %d \n",status);
        if(status == -1)
        {
                exit(0);
        }

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
	int pid = atoi(buffer);
	cgrules_search_classify(pid);
        }
	
    } while (!must_stop);

    /* cleanup */
   CHECK((mqd_t)-1 != mq_close(mq));
   CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));
   hdestroy_r(cgrules_slice_dict);
   free(cgrules_slice_dict);
   return 0;
}

static char *cgroup_basename(const char *path)
{
	char *base;
	char *tmp_string;

	tmp_string = strdup(path);

	if (!tmp_string)
		return NULL;

	base = strdup(basename(tmp_string));

	free(tmp_string);

	return base;
}
int cgrules_classify(int pid,char *slice)
{
  if(slice != NULL)
        {
                char str[20];
		ENTRY *slice_ep,slice_e;
                sprintf(str, "%d", pid);
                FILE *f;
                char path[FILENAME_MAX];
		slice_e.key=slice;
		hsearch_r(slice_e, FIND,&slice_ep,slice_map);
	        if(slice_ep != NULL)
                {
			sprintf(path, "%s/cgroup.procs",((char*)slice_ep->data));
			 f = fopen(path, "w");
                	 if (!f)
                         	return -1;
                	fputs(str,f);
                	fclose(f);

                }
		else
		{
			return -1;
		}
                        return 1;
        }
  else if(slice == NULL)
  {
	return -1;
  } 
}



int cgrules_search_classify(int pid)
{

ENTRY e;
ENTRY *ep;
char *procname;
int status=0;
int ret = cgroup_get_procname_from_procfs(pid, &procname);
if (ret == 0)
{
        e.key = procname;
        hsearch_r(e, FIND,&ep,cgrules_slice_dict);
        
	if(ep != NULL)
	{
	status = cgrules_classify(pid,((char*)(ep->data)));
        printf("pid : %d, Process Name : %s Slice : %s\n",pid,procname,(char*)(ep->data));
	return 1;
	}
	else if(ep == NULL)
        {
	e.key = cgroup_basename(procname);
	hsearch_r(e, FIND,&ep,cgrules_slice_dict);
        	if(ep != NULL)
        	{
        		status = cgrules_classify(pid,((char*)(ep->data)));
        		printf("pid : %d, Process Name : %s Slice : %s\n",pid,procname,(char*)(ep->data));
			return 1;
        	}

	}
	if (status == -1)
		
        	printf("pid : %d, Process Name : %s Slice : Null \n",pid,procname);
		return -1;
}

}

