#define _GNU_SOURCE
#include <time.h>
# include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include "proc_probe.h"
#define MAX_LINE_SIZE 1024
int live_check(char *);
void poll_process_liveness(char*,char*);


char *trim_white_space(char *str)
{
	if (str == NULL)
		pthread_exit(NULL);
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

void* parse_lines(void * arg)
{
		char* line = (char*)arg;
	        const char delimeter[3] = ":";
		char *binary,*slice;
                binary = strdup(trim_white_space(strtok(line, delimeter)));
                slice  = strdup(trim_white_space(strtok(0, delimeter)));
		poll_process_liveness(binary,slice);
}

void poll_process_liveness(char * name , char * slice)
{
        int ret = -1,status;
        int len;
        FILE *f;
        char path[FILENAME_MAX];
	char *Process_name;
        char buf[4092];
	int correct_slice = 0;
	time_t start_seconds,end_seconds,total_time;

	start:
		status = live_check(name);

		ret = cgroup_get_procname_from_procfs(status,&Process_name);
		if (ret == 0)
		{
			name = strdup(Process_name);
		}
		printf("Process %s has started at : %ld\n",name,start_seconds = time(NULL));
        	sprintf(path, "/proc/%d/cgroup", status);
	continue_polling:
		f = fopen(path, "re");
        	if (!f)
		{
			printf("Process %s file %s doesnt exist\n",name,path);
	   		goto start;
		}
        while(fgets(buf, sizeof(buf), f))
	{
                if(strstr(buf,slice)!= NULL) {
			correct_slice = 1;
			break;	
		}
        }
	fclose(f);
	sleep(6);
	f = fopen(path,"re");
        if (!f)
	{
	   total_time=0;
           correct_slice=0;
           printf("Process %s has stopped at : %ld \n",name,time(NULL));
           goto start;
	}
	else if(correct_slice == 1)
	{	
		end_seconds = time(NULL);
		total_time = end_seconds - start_seconds;
		printf("Process %s is in the %s slice for %ld Seconds \n",name,slice,total_time);
		fclose(f);
		goto continue_polling;
	}
	else
	{
		printf("Process %s is not in the Intended Slice\n",name);
		fclose(f);
		goto continue_polling;
	}
	fclose(f);
}


int live_check(char * name)
{
	FILE *fp;
        char  command[MAX_LINE_SIZE],base_command[MAX_LINE_SIZE];
        char buf[MAX_LINE_SIZE];
	char * pid,*Process_name;

  	sprintf(command,"ps -aef | grep \"%s\" | grep -v \"grep\" | awk \'{print $2,$8}\'",name);

	while(1)
	{
	       if ((fp = popen(command, "r")) == NULL)
	       {
               printf("Error opening pipe!\n");
	       }
       		
		while(fgets(buf,MAX_LINE_SIZE, fp) != NULL) {

			pid = strdup(strtok(buf," "));
			Process_name  = strdup((strtok(0, " ")));
			fclose(fp);
			return atoi(pid);
       		}
	
	fclose(fp);
	sleep(10);
	}
}
