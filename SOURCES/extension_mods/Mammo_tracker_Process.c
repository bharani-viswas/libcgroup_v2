#define _GNU_SOURCE
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <string.h>
#include <linux/netlink.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <linux/un.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <pthread.h> 
#include "Process_classifier.h"

#define QUEUE_NAME  "/cgroup_queue"
#define SOCKET_PATH "/var/run/viswas.socket"

#define SEND_MESSAGE_LEN (NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op)))
#define RECV_MESSAGE_LEN (NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(struct proc_event)))

#define SEND_MESSAGE_SIZE (NLMSG_SPACE(SEND_MESSAGE_LEN))
#define RECV_MESSAGE_SIZE (NLMSG_SPACE(RECV_MESSAGE_LEN))

#define BUFF_SIZE (max(max(SEND_MESSAGE_SIZE, RECV_MESSAGE_SIZE), 1024))
#define MIN_RECV_SIZE (min(SEND_MESSAGE_SIZE, RECV_MESSAGE_SIZE))

#define PROC_CN_MCAST_LISTEN (1)
#define PROC_CN_MCAST_IGNORE (2)
#define max(x,y) ((y)<(x)?(x):(y))
#define min(x,y) ((y)>(x)?(x):(y))

#define MAX_SIZE    1024
#define MSG_STOP    "exit"

mqd_t mq;
char buffer[MAX_SIZE];

#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \

uid_t socket_user = -1;
gid_t socket_group = -1;


//int process_event(const struct proc_event *ev, const int type);
static int handle_msg(struct cn_msg *cn_hdr);
static int receive_netlink_msg(int sk_nl);
static int netlink_socket_process_msg(void);
static void receive_unix_domain_msg(int sk_unix);
int process_event_exec(const int pid);



static int netlink_socket_process_msg(void)
{
	int sk_nl = 0, sk_unix = 0, sk_max;
	struct sockaddr_nl my_nla;
	char buff[BUFF_SIZE];
	int rc = -1;
	struct nlmsghdr *nl_hdr;
	struct cn_msg *cn_hdr;
	enum proc_cn_mcast_op *mcop_msg;
	struct sockaddr_un saddr;
	fd_set fds, readfds;
	sigset_t sigset;

	sk_nl = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if (sk_nl == -1) {
		printf("Error: error opening netlink socket: %s\n",strerror(errno));
		return rc;
	}

	my_nla.nl_family = AF_NETLINK;
	my_nla.nl_groups = CN_IDX_PROC;
	my_nla.nl_pid = getpid();
	my_nla.nl_pad = 0;

	if (bind(sk_nl, (struct sockaddr *)&my_nla, sizeof(my_nla)) < 0) {
		printf("Error: error binding netlink socket: %s\n",strerror(errno));
		goto close_and_exit;
	}

	nl_hdr = (struct nlmsghdr *)buff;
	cn_hdr = (struct cn_msg *)NLMSG_DATA(nl_hdr);
	mcop_msg = (enum proc_cn_mcast_op*)&cn_hdr->data[0];
	memset(buff, 0, sizeof(buff));
	*mcop_msg = PROC_CN_MCAST_LISTEN;

	/* fill the netlink header */
	nl_hdr->nlmsg_len = SEND_MESSAGE_LEN;
	nl_hdr->nlmsg_type = NLMSG_DONE;
	nl_hdr->nlmsg_flags = 0;
	nl_hdr->nlmsg_seq = 0;
	nl_hdr->nlmsg_pid = getpid();

	/* fill the connector header */
	cn_hdr->id.idx = CN_IDX_PROC;
	cn_hdr->id.val = CN_VAL_PROC;
	cn_hdr->seq = 0;
	cn_hdr->ack = 0;
	cn_hdr->len = sizeof(enum proc_cn_mcast_op);
	printf("Sending netlink message len=%d, cn_msg len=%d\n",nl_hdr->nlmsg_len, (int) sizeof(struct cn_msg));
	if (send(sk_nl, nl_hdr, nl_hdr->nlmsg_len, 0) != nl_hdr->nlmsg_len) {
		printf("Error: failed to send netlink message (mcast ctl op): %s\n",strerror(errno));
		goto close_and_exit;
	}

	sk_unix = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sk_unix < 0) {
		printf("Error creating UNIX socket: %s\n",strerror(errno));
		goto close_and_exit;
	}
	memset(&saddr, 0, sizeof(saddr));
	saddr.sun_family = AF_UNIX;
	strcpy(saddr.sun_path, SOCKET_PATH);
	unlink(SOCKET_PATH);
	if (bind(sk_unix, (struct sockaddr *)&saddr,sizeof(saddr.sun_family) + strlen(SOCKET_PATH)) < 0)
	{
		printf("Error binding UNIX socket %s: %s\n",SOCKET_PATH, strerror(errno));
		goto close_and_exit;
	}
	if (listen(sk_unix, 1) < 0) {
		printf("Error listening on UNIX socket %s: %s\n",SOCKET_PATH, strerror(errno));
		goto close_and_exit;
	}

	/* change the owner */
	if (chown(SOCKET_PATH, socket_user, socket_group) < 0) {
		printf("Error changing %s socket owner: %s\n",SOCKET_PATH, strerror(errno));
		goto close_and_exit;
	}
	printf("Socket %s owner successfully set to %d:%d\n",SOCKET_PATH, (int) socket_user,(int) socket_group);

	if (chmod(SOCKET_PATH, 0660) < 0) {
		printf("Error changing %s socket permissions: %s\n",SOCKET_PATH, strerror(errno));
		goto close_and_exit;
	}

	FD_ZERO(&readfds);
	FD_SET(sk_nl, &readfds);
	FD_SET(sk_unix, &readfds);
	if (sk_nl < sk_unix)
		sk_max = sk_unix;
	else
		sk_max = sk_nl;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR2);
	for(;;) {
		/*
		 * For avoiding the deadlock and "Interrupted system call"
		 * error, restrict the effective range of SIGUSR2 signal.
		 */
		sigprocmask(SIG_UNBLOCK, &sigset, NULL);
		sigprocmask(SIG_BLOCK, &sigset, NULL);

		memcpy(&fds, &readfds, sizeof(fd_set));
		if (select(sk_max + 1, &fds, NULL, NULL, NULL) < 0) {
			printf("Selecting error: %s\n", strerror(errno));
			goto close_and_exit;
		}
		if (FD_ISSET(sk_nl, &fds)) {
			if (receive_netlink_msg(sk_nl))
				break;
		}
		if (FD_ISSET(sk_unix, &fds))
			receive_unix_domain_msg(sk_unix);
	}

	close_and_exit:
		if (sk_nl >= 0)
			close(sk_nl);
		if (sk_unix >= 0)
			close(sk_unix);
		return rc;
}

static void receive_unix_domain_msg(int sk_unix)
{
	int flags;
	int fd_client;
	pid_t pid;
	struct sockaddr_un caddr;
	socklen_t caddr_len;
	struct stat buff_stat;
	char path[FILENAME_MAX];

	caddr_len = sizeof(caddr);
	fd_client = accept(sk_unix, (struct sockaddr *)&caddr, &caddr_len);
	if (fd_client < 0) {
		printf("Warning: 'accept' command error: %s\n",strerror(errno));
		return;
	}
	if (read(fd_client, &pid, sizeof(pid)) < 0) {
		printf("Warning: 'read' command error: %s\n",strerror(errno));
		goto close;
	}
	sprintf(path, "/proc/%d", pid);
	if (stat(path, &buff_stat)) {
		printf("Warning: there is no such process (PID: %d)\n",pid);
		goto close;
	}
	if (read(fd_client, &flags, sizeof(flags)) < 0) {
		printf("Warning: error reading daemon socket: %s\n",strerror(errno));
		goto close;
	}
close:
	close(fd_client);
	return;
}


static int receive_netlink_msg(int sk_nl)
{
	char buff[BUFF_SIZE];
	size_t recv_len;
	struct sockaddr_nl from_nla;
	socklen_t from_nla_len;
	struct nlmsghdr *nlh;
	struct cn_msg *cn_hdr;

	memset(buff, 0, sizeof(buff));
	from_nla_len = sizeof(from_nla);
	recv_len = recvfrom(sk_nl, buff, sizeof(buff), 0, (struct sockaddr *)&from_nla, &from_nla_len);
	if (recv_len == ENOBUFS) {
		printf("ERROR: NETLINK BUFFER FULL, MESSAGE DROPPED!\n");
		return 0;
	}
	if (recv_len < 1)
		return 0;

	if(from_nla_len != sizeof(from_nla))
	{
		printf("Bad address size reading netlink socket\n");
		return 0;
	}
	if (from_nla.nl_groups != CN_IDX_PROC || from_nla.nl_pid != 0)
		return 0;

	nlh = (struct nlmsghdr *)buff;
	while (NLMSG_OK(nlh, recv_len)) {
		cn_hdr = NLMSG_DATA(nlh);
		if (nlh->nlmsg_type == NLMSG_NOOP) {
			nlh = NLMSG_NEXT(nlh, recv_len);
			continue;
		}
		if ((nlh->nlmsg_type == NLMSG_ERROR) || (nlh->nlmsg_type == NLMSG_OVERRUN))
			break;
		if (handle_msg(cn_hdr) < 0)
			return 1;
		if (nlh->nlmsg_type == NLMSG_DONE)
			break;
		nlh = NLMSG_NEXT(nlh, recv_len);
	}
	return 0;
}

static int handle_msg(struct cn_msg *cn_hdr)
{
	struct proc_event *ev;
	int ret = 0;

	/* Get the event data.  We only care about two event types. */
	ev = (struct proc_event*)cn_hdr->data;
	switch (ev->what) {
	case PROC_EVENT_UID:
		//ret = process_event(ev, PROC_EVENT_UID);
		break;
	case PROC_EVENT_GID:
		//ret = process_event(ev, PROC_EVENT_GID);
		break;
	case PROC_EVENT_FORK:
		//ret = process_event(ev, PROC_EVENT_FORK);
		break;
	case PROC_EVENT_EXIT:
		//ret = process_event(ev, PROC_EVENT_EXIT);
		break;
	case PROC_EVENT_EXEC:
		memset(buffer, 0, MAX_SIZE);
		snprintf(buffer, MAX_SIZE, "%d", ev->event_data.exec.process_pid);
	        CHECK(0 <= mq_send(mq, buffer, MAX_SIZE, 0));
		break;
	default:
		break;
	}

	return ret;
}


void main()
{
    	struct mq_attr attr;
	pthread_t ptid; 
	
    	/* initialize the queue attributes */
    	attr.mq_flags = 0;
    	attr.mq_maxmsg = 10;
    	attr.mq_msgsize = MAX_SIZE;
    	attr.mq_curmsgs = 0;

	pthread_create(&ptid, NULL,&Process_clasifier, NULL); 
    	/* create the message queue */
    	mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
	CHECK((mqd_t)-1 != mq);
	netlink_socket_process_msg();
        CHECK(0 <= mq_send(mq,"exit", MAX_SIZE, 0));
	pthread_join(ptid, NULL); 
	pthread_exit(NULL); 

}

