#include "afs_client.h"

#include "afs_protocol.h"

#include <linux/net.h>

static struct socket *sock = NULL;

int afs_init_connection(void)
{
	int err;
	int portnum;
	int ret;
	struct sockaddr_in s_addr;

	err = sock_create_kern(&init_net, PF_INET, SOCK_STREAM, IPPROTO_TCP,
			       &sock);
	if (err < 0) {
		pr_err("socket not created\n");
		return -1;
	}
	pr_info("socket is created\n");

	portnum = PORT;
	memset(&s_addr, 0, sizeof(s_addr));

	s_addr = { .sin_family = AF_INET,
		   .sin_port = htons(portnum),
		   .sin_addr = { .s_addr = htonl(INADDR_LOOPBACK) } };

	ret = sock->ops->connect(sock, (struct sockaddr *)&s_addr,
				 sizeof(s_addr), 0);
	if (ret != 0) {
		pr_err("connect error!\n");
		return -1;
	}
	pr_info("connect ok!\n");
	return 0;
}

int afs_remote_create(int dir_handle, char name[MAX_NAME_LEN])
{
	afs_init_connection();
	return 0;
}