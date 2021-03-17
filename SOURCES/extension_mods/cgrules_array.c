#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_LINE_SIZE 1024

int countLines(FILE *file)
{
int lines;
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

int main(void) {
	int i = 0;
    	FILE *file;
	const char delimeter[3] = ":"; 
    	file = fopen("./files.txt", "r");
	int count = countLines(file);
	char line[MAX_LINE_SIZE];
	char *slice[count];
	char* binary,*slice_name;
	printf("Number of lines : %d\n",count);

    while(fgets(line, sizeof(line), file)!=NULL) {
	if(strcmp(line,"\n") != 0)
	{
		binary = strtok(line, delimeter);
		printf(" %s\n", binary); 
		        slice_name = strtok(0, delimeter); 
		                printf(" %s\n", slice_name);
	
	}
    }
    
//    for(i=0;i<count;i++)
//	{
//	printf("%d --> %s",i,slice[i]);
//	}


    fclose(file);
    return 0;
}
