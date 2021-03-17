#define _GNU_SOURCE
#include<errno.h>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <ctype.h>
#define MAX_LINE_SIZE 1024
int slices_count=0;
struct hsearch_data * slice_map;
struct hsearch_data * cgrules_slice_dict;

int cgrules_configure(void);
int create_slice_map(void);

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

 
int cgrules_configure(void)
{
        ENTRY e, *ep;
        char line[MAX_LINE_SIZE];
	FILE * cgrules_conf_file;
        const char delimeter[3] = ":";
        if(!(cgrules_conf_file = fopen("/etc/cgrules.conf", "r")))
        {
                printf("/etc/cgrules.conf file doesnt exist");
		return -1;
        }

	int cgrules_entries = countLines(cgrules_conf_file);
	cgrules_slice_dict = calloc(1,sizeof(struct hsearch_data));
	hcreate_r(cgrules_entries,cgrules_slice_dict);
	
	while(fgets(line, sizeof(line),cgrules_conf_file)!=NULL){
        if(strcmp(line,"\n") != 0){
	        //Keys represent the binaries and data (values) represent the slice names
                e.key = strdup(trim_white_space(strtok(line, delimeter)));
                e.data = strdup(trim_white_space(strtok(0, delimeter)));
		hsearch_r(e, ENTER,&ep,cgrules_slice_dict);
                if (ep == NULL) {
                    fprintf(stderr, "entry failed\n");
		    fclose(cgrules_conf_file);
                    return -1;
                }
        }
   	}
fclose(cgrules_conf_file);
return 1;
}


int create_slice_map(void)
{
	ENTRY e, *ep;
        int slice_count;
	FILE* fp,*proc_mount;
        char buffer[MAX_LINE_SIZE];
        char command[MAX_LINE_SIZE];
        char command_line[MAX_LINE_SIZE];
        struct mntent *ent = NULL;
        struct mntent *temp_ent = NULL;
        char mntent_buffer[MAX_LINE_SIZE];
        proc_mount = fopen("/proc/mounts", "re");
        if (proc_mount == NULL) {
                printf("Error: cannot open /proc/mounts: %s\n",strerror(errno));
                return -1;
        }
        temp_ent = (struct mntent *) malloc(sizeof(struct mntent));
        if (!temp_ent) {
                fclose(proc_mount);
                printf("Error: cannot allocate memory: %s\n",strerror(errno));
                return -1;
        }

        while ((ent = getmntent_r(proc_mount, temp_ent,mntent_buffer,sizeof(mntent_buffer))) != NULL) {
                if (strcmp(ent->mnt_type, "cgroup2") == 0) {
                        sprintf(command_line,"find %s -type d -path \"*.slice\" | wc -l",ent->mnt_dir);
                        sprintf(command,"find %s -type d -path \"*.slice\"",ent->mnt_dir);
                        if ((fp = popen(command_line, "r")) == NULL)
                        {
                                printf("Error opening pipe!\n");
                                return -1;
                        }
			else if(fgets(buffer,MAX_LINE_SIZE,fp) != NULL)
			{
			slice_count = atoi(buffer);
			}

			 printf("Slice count : %d \n",slice_count);

			 slice_map = calloc(1,sizeof(struct hsearch_data));
			 hcreate_r(slice_count,slice_map);

                        if ((fp = popen(command, "r")) == NULL)
                        {
                                printf("Error opening pipe!\n");
                                return -1;
                        }


                         while (fgets(buffer,MAX_LINE_SIZE,fp) != NULL) {
                           if(strcmp(buffer,"\n") != 0){
                                //Keys represent the slice name and  data (values) represent the absolute slice path
                                e.key = strdup(basename(trim_white_space(buffer)));
                                e.data = strdup(trim_white_space(buffer));
                                hsearch_r(e, ENTER,&ep,slice_map);
                                if (ep != NULL) {
                                        printf("Inserting :  %s --> %s\n",(char*)ep->key,(char*)ep->data);
                                }
				else
				{
					printf("Error Entry failed\n");
                                        return -1;

				}
                        }
                }
		fclose(fp);
                break;
                }

        }
fclose(proc_mount);
return 1;
}
