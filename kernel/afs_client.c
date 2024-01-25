#include "afs_client.h"

#include "afs_protocol.h"

#include <net/sock.h>
#include <linux/inet.h>
#include <linux/socket.h>

static struct socket *sock = NULL;

static int afs_init_connection(void)
{
	int err;
	int portnum;
	int ret;
	struct sockaddr_in s_addr;

	err = sock_create_kern(&init_net, PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
	if (err < 0) {
		pr_err("socket not created\n");
		return -1;
	}
	pr_info("socket is created\n");

	portnum = PORT;
	memset(&s_addr, 0, sizeof(s_addr));

	s_addr = (struct sockaddr_in){ .sin_family = AF_INET,
				       .sin_port = htons(portnum),
				       .sin_addr = { .s_addr = htonl(INADDR_LOOPBACK) } };

	ret = sock->ops->connect(sock, (struct sockaddr *)&s_addr, sizeof(s_addr), 0);
	if (ret != 0) {
		pr_err("connect error!\n");
		sock_release(sock);
		pr_info("socket released\n");
		return -1;
	}
	pr_info("connect ok!\n");
	return 0;
}

static int afs_recv_buffer(void *buffer, size_t len)
{
	int ret;
	struct kvec vec = { 0 };
	struct msghdr msg = { 0 };

	vec.iov_base = buffer;
	vec.iov_len = len;

	ret = kernel_recvmsg(sock, &msg, &vec, 1, vec.iov_len, MSG_WAITALL);
	if (ret < 0) {
		pr_err("kernel_recvmsg error!");
		return ret;
	} else if (ret != len) {
		pr_warn("received less than expected");
	}
	pr_info("successfull receive!");

	return 0;
}

static int afs_send_buffer(void *message, size_t len)
{
	int ret;
	struct kvec vec;
	struct msghdr msg = { 0 };

	pr_info("trying to send to server %zu bytes\n", len);

	vec.iov_base = message;
	vec.iov_len = len;

	ret = kernel_sendmsg(sock, &msg, &vec, 1, len);
	if (ret < 0) {
		pr_err("kernel_sendmsg error!");
		return ret;
	} else if (ret != len) {
		pr_warn("sent less than expected");
	}
	pr_info("successfull send!");
	return 0;
}

static void afs_cleanup_connection(void)
{
	sock_release(sock);
	pr_info("socket released\n");
}

int afs_remote_create(int dir_handle, const char name[MAX_NAME_LEN])
{
	struct afs_response res = {};
	/* clang-format off */
	struct afs_request req = { 
        .type = AFS_CREATE,
		.args = { .as_create = { .dir = dir_handle }}
    };
	/* clang-format on */

	memcpy(req.args.as_create.name, name, MAX_NAME_LEN);

	afs_init_connection();

	pr_info("sending remote create request\n");

	afs_send_buffer(&req, sizeof(req));

	afs_recv_buffer(&res, sizeof(res));

	afs_cleanup_connection();

	if (res.status == AFS_OK) {
		pr_info("received handle %d\n", res.body.as_create);
		return res.body.as_create;
	}

	pr_warn("remote create returned error\n");
	return 0;
}

int afs_remote_lookup(int dir_handle, const char name[MAX_NAME_LEN], struct afs_remote_lookup_result *res)
{
	struct afs_response response = {};
	/* clang-format off */
	struct afs_request req = { 
        .type = AFS_LOOKUP,
		.args = { .as_lookup = { .dir = dir_handle }}
    };
	/* clang-format on */

	memcpy(req.args.as_lookup.name, name, MAX_NAME_LEN);

	afs_init_connection();

	pr_info("sending remote lookup request\n");

	afs_send_buffer(&req, sizeof(req));

	afs_recv_buffer(&response, sizeof(response));

	afs_cleanup_connection();

	if (response.status == AFS_OK) {
		pr_info("received handle %d\n", response.body.as_lookup.handle);
		res->handle = response.body.as_lookup.handle;
		if (response.body.as_lookup.type == AFS_DIR) {
			res->umode = S_IFDIR;
		} else {
			res->umode = S_IFREG;
		}
		return 1;
	}

	pr_warn("remote lookup returned error\n");
	return 0;
}

int afs_remote_readdir(int dir_handle, struct afs_remote_readdir_result res[MAX_FILES_PER_READDIR])
{
	struct afs_response response = {};
	/* clang-format off */
	struct afs_request req = { 
        .type = AFS_READDIR,
		.args = { .as_readdir = { .dir = dir_handle }}
    };
	/* clang-format on */
	int i;

	afs_init_connection();

	pr_info("sending remote readdir request\n");

	afs_send_buffer(&req, sizeof(req));

	afs_recv_buffer(&response, sizeof(response));

	afs_cleanup_connection();

	if (response.status == AFS_OK) {
		pr_info("received handle %d\n", response.body.as_lookup.handle);

		for (i = 0; i < MAX_FILES_PER_READDIR; ++i) {
			res[i].handle = response.body.as_readdir[i].handle;
			if (res[i].handle == 0) {
				break;
			}

			if (response.body.as_readdir[i].type == AFS_DIR) {
				res[i].ftype = DT_DIR;
			} else {
				res[i].ftype = DT_REG;
			}

			memcpy(res[i].name, response.body.as_readdir[i].name, MAX_NAME_LEN);
			res[i].name_len = strlen(response.body.as_readdir[i].name);
		}

		return i;
	}

	pr_warn("remote lookup returned error\n");
	return -1;
}