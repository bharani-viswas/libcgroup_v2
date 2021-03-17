#include <dirent.h>
#include <errno.h>
#include <mntent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fts.h>
#include <ctype.h>
#include <pwd.h>
#include <libgen.h>
#include <assert.h>
#include <linux/un.h>
#include <grp.h>

#define TASK_COMM_LEN 16
int last_errno;
int cgroup_get_procname_from_procfs(pid_t, char **);
static int cg_get_procname_from_proc_status(pid_t, char **);
static int cg_get_procname_from_proc_cmdline(pid_t,const char *, char **);


static int cg_get_procname_from_proc_status(pid_t pid, char **procname_status)
{
	int ret = -1;
	int len;
	FILE *f;
	char path[FILENAME_MAX];
	char buf[4092];

	sprintf(path, "/proc/%d/status", pid);
	f = fopen(path, "re");
	if (!f)
		return -1;

	while (fgets(buf, sizeof(buf), f)) {
		if (!strncmp(buf, "Name:", 5)) {
			len = strlen(buf);
			if (buf[len - 1] == '\n')
				buf[len - 1] = '\0';
			*procname_status = strdup(buf + strlen("Name:") + 1);
			if (*procname_status == NULL) {
				ret = -1;
				break;
			}
			ret = 0;
			break;
		}
	}
	fclose(f);
	return ret;
}

static int cg_get_procname_from_proc_cmdline(pid_t pid,const char *pname_status, char **pname_cmdline)
{
	FILE *f;
	int ret = -1;
	int c = 0;
	int len = 0;
	char path[FILENAME_MAX];
	char buf_pname[FILENAME_MAX];
	char buf_cwd[FILENAME_MAX];

	memset(buf_cwd, '\0', sizeof(buf_cwd));
	sprintf(path, "/proc/%d/cwd", pid);
	if (readlink(path, buf_cwd, sizeof(buf_cwd)) < 0)
		return -1;

	sprintf(path, "/proc/%d/cmdline", pid);
	f = fopen(path, "re");
	if (!f)
		return -1;

	while (c != EOF) {
		c = fgetc(f);
		if ((c != EOF) && (c != '\0') && (len < FILENAME_MAX - 1)) {
			buf_pname[len] = c;
			len++;
			continue;
		}
		buf_pname[len] = '\0';

		if (len == FILENAME_MAX - 1)
			while ((c != EOF) && (c != '\0'))
				c = fgetc(f);

		/*
		 * The taken process name from /proc/<pid>/status is
		 * shortened to 15 characters if it is over. So the
		 * name should be compared by its length.
		 */
		if (strncmp(pname_status, basename(buf_pname),
						TASK_COMM_LEN - 1)) {
			len = 0;
			continue;
		}
		if (buf_pname[0] == '/') {
			*pname_cmdline = strdup(buf_pname);
			if (*pname_cmdline == NULL) {
				last_errno = errno;
				ret = -1;
				break;
			}
			ret = 0;
			break;
		} else {
			strcat(buf_cwd, "/");
			strcat(buf_cwd, buf_pname);
			if (!realpath(buf_cwd, path)) {
				last_errno = errno;
				ret = -1;
				break;
			}
			*pname_cmdline = strdup(path);
			if (*pname_cmdline == NULL) {
				last_errno = errno;
				ret = -1;
				break;
			}
			ret = 0;
			break;
		}
	}
	fclose(f);
	return ret;
}

  

int cgroup_get_procname_from_procfs(pid_t pid, char **procname)
{
	int ret;
	char *pname_status;
	char *pname_cmdline;
	char path[FILENAME_MAX];
	char buf[FILENAME_MAX];

	ret = cg_get_procname_from_proc_status(pid, &pname_status);
	if (ret)
		return ret;


	memset(buf, '\0', sizeof(buf));
	sprintf(path, "/proc/%d/exe", pid);
	if (readlink(path, buf, sizeof(buf)) < 0) {
		*procname = pname_status;
		return 0;
	}
	if (!strncmp(pname_status, basename(buf), TASK_COMM_LEN - 1)) {
		free(pname_status);
		*procname = strdup(buf);
		if (*procname == NULL) {
			last_errno = errno;
			return -1;
		}
		return 0;
	}

	ret = cg_get_procname_from_proc_cmdline(pid, pname_status,
						    &pname_cmdline);
	if (!ret) {
		*procname = pname_cmdline;
		free(pname_status);
		return 0;
	}

	free(pname_status);
	*procname = strdup(buf);
	if (*procname == NULL) {
		last_errno = errno;
		return -1;
	}
	return 0;
}







