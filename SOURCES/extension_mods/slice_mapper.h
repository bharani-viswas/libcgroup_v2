#include<stdio.h>
#include<string.h>
#include<errno.h>
#include <stdlib.h>
#include <mntent.h>
#define MAX_LINE_SIZE 1024
#include "cgrules_dict.h"

char **slices;
int slices_count=0;
struct hsearch_data * slice_map;

int create_slice_map(void)
{	
	int increment=5;
	slices = (char**)malloc(increment*sizeof(char*));
	FILE* fp;
	char buf[1024];
	char command[1024];
	struct mntent *ent = NULL;
	struct mntent *temp_ent = NULL;
	char mntent_buffer[MAX_LINE_SIZE];
	FILE*proc_mount = fopen("/proc/mounts", "re");
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
			sprintf(command,"find %s -type d -path \"*.slice\"",ent->mnt_dir);
                        if ((fp = popen(command, "r")) == NULL)
                        {
                                printf("Error opening pipe!\n");
                                return -1;
                        }

                         while (fgets(buf,MAX_LINE_SIZE, fp) != NULL) {
				
			   if(strcmp(buf,"\n") != 0){
	                   	//Keys represent the slice name and  data (values) represent the absolute slice path
                        	e.key = strdup(basename(buf);
                        	e.data = strdup(buf);
                		hsearch_r(e, ENTER,&ep,cgrules_slice_dict);
                		if (ep == NULL) {
                    			fprintf(stderr, "entry failed\n");
                    			return -1;
                		}
                	}
                 }
		break;
		}

	}
fclose(fp);
fclose(proc_mount);
return 1;
}

static void create_slice_map() {
    char *bname;
    ENTRY e, *ep;
    slice_map = calloc(1,sizeof(struct hsearch_data));
    hcreate_r(slices_count,slice_map);
	for(int i=0;i<slices_count;i++)
	{	
		if(strcmp(slices[i],"\0") != 0)
		{
                //Keys represent the slice name and  data (values) represent the absolute slice path
                	e.key = strdup(basename(slices[i]));
                	e.data = strdup(slices[i]);
                hsearch_r(e, ENTER,&ep,cgrules_slice_dict);
                if (ep == NULL) {
                    fprintf(stderr, "entry failed\n");
                    return -1;
                }

		}
	}
     
}

find_cgroup_mounts();
for(int i=0;i<slices_count;i++)
	printf("%s",slices[i]);
return 0;
}
