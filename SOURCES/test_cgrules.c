#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <ctype.h>
#define MAX_LINE_SIZE 1024



char *trim_white_space(char *str)
{
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

unsigned int countLines(FILE *file)
{
    unsigned int  lines = 0;
    int           c = '\0';
    int           pc = '\n';

    while (c = fgetc(file), c != EOF)
    {
        if (c == '\n'  &&  pc != '\n')
            lines++;
        pc = c;
    }
    if (pc != '\n')
        lines++;
  rewind(file);
    return lines;
}


 
int main(void)
{
        char line[MAX_LINE_SIZE];
	char*binary,*slice;
	FILE * cgrules_conf_file,*file_stream;
        const char delimeter[3] = ":";
	char  command[MAX_LINE_SIZE];
	char buf[MAX_LINE_SIZE];
        if(!(cgrules_conf_file = fopen("/etc/cgrules.conf", "r")))
        {
                printf("/etc/cgrules.conf file doesnt exist");
        }

	
	while(fgets(line, sizeof(line),cgrules_conf_file)!=NULL) {
        	if(strcmp(line,"\n") != 0){
                	binary = strdup(trim_white_space(strtok(line, delimeter)));
                	slice  = strdup(trim_white_space(strtok(0, delimeter)));
			sprintf(command,"journalctl -u cgrules.service \| grep \"%s Slice : %s\"",binary,slice);
			if ((fp = popen(command, "r")) == NULL) 
			{
     				printf("Error opening pipe!\n");
 				return -1;
			}
			
			 while (fgets(buf,MAX_LINE_SIZE, fp) != NULL) {
        			printf("OUTPUT: %s", buf);
    			 }	

                }
        }
 


fclose(cgrules_conf_file);
}

