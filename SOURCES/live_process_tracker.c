#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "Process_status.h"
#define MAX_LINE_SIZE 1024

// Returns nonzero iff line is a string containing only whitespace (or is empty)
int isBlank (char * line)
{
  char * ch;
  int is_blank = -1;

  // Iterate through each character.
  for (ch = line; *ch != '\0' && *ch != '\n' && *ch != -1; ++ch)
  {
    if (!isspace(*ch))
    {
      // Found a non-whitespace character.
      is_blank = 0;
      break;
    }
  }

  return is_blank;
}

//In case of threaded calls try to reopen instead of using rewind
int countLines(FILE *file)
{
int lines=0;
char line[MAX_LINE_SIZE];
char *str;
while(fgets(line,MAX_LINE_SIZE, file) != NULL) {
        str = line;
    if(strlen(line) > 1) {

              while(isspace((unsigned char)*str)) str++;
               if(*str != 0)  // All spaces?
                  lines++;
    }
}
rewind(file);
return lines;
}



int main(void)
{	
	FILE *fp;
	char line[MAX_LINE_SIZE];
        FILE * cgrules_conf_file,*file_stream;
        char  command[MAX_LINE_SIZE];
        char buf[MAX_LINE_SIZE];
        if(!(cgrules_conf_file = fopen("/etc/cgrules.conf", "r")))
        {
                printf("/etc/cgrules.conf file doesnt exist");
        }
	int thread_count = countLines(cgrules_conf_file);
	thread_count = thread_count+20;
	pthread_t process_threads[thread_count];
//	Prefer stack to avoid leaks in case of SEG faults
	char* thread_args[thread_count];	
	int thread_order=0;
        while(fgets(line, sizeof(line),cgrules_conf_file)!=NULL) {
                if(isBlank(line) == 0){
			thread_args[thread_order] = strdup(line);	
			pthread_create(&process_threads[thread_order],NULL,parse_lines,thread_args[thread_order]);
			thread_order++;
                }
        }

	for(int temp_id=0;temp_id<thread_order;temp_id++)
	{
		pthread_join(process_threads[temp_id],NULL);
	}
	fclose(cgrules_conf_file);
}

