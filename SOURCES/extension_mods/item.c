#include<stdio.h>
#include<string.h>
#include<errno.h>
#include <stdlib.h>
#include <mntent.h>
#define MAX_LINE_SIZE 1024
char **slices;
int slices_count=0;
int find_cgroup_mounts(void)
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
				if(slices_count == increment){
				slices = (char**)realloc(slices,(slices_count+increment)*sizeof(char*));
					increment = increment * 2;
				}
				else{
                                        slices[slices_count]=strdup(buf);
					slices_count++;
				}	
                         }
		break;
		}

	}
fclose(fp);
fclose(proc_mount);
return 1;
}

static void slice_mapper(char *path) {
    char *bname;
    char *path2 = strdup(path);
    bname = basename(path2);
     
}

int main()
{
/*FILE *fp;
char command[1024];
char buf[1024];
char binary[] = "MammoServer.lnx";
char slice[] = "Acquisition.slice";
sprintf(command,"journalctl -u cgrules.service | grep \"*Process Name : %s*Slice : %s*\"",binary,slice);

                        if ((fp = popen(command, "r")) == NULL)
                        {
                                printf("Error opening pipe!\n");
                                return -1;
                        }

                         while (fgets(buf,MAX_LINE_SIZE, fp) != NULL) {
                                printf("OUTPUT: %s", buf);
                         }

*/
find_cgroup_mounts();
for(int i=0;i<slices_count;i++)
	printf("%s",slices[i]);
return 0;
}
