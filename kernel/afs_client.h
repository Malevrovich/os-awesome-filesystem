#ifndef _AFS_CLIENT_H_
#define _AFS_CLIENT_H_

#include <linux/fs.h>

#include "afs_config.h"

int afs_remote_create(int dir_handle, const char name[MAX_NAME_LEN]);

struct afs_remote_lookup_result {
	int handle;
	umode_t umode;
};

int afs_remote_lookup(int dir_handle, const char name[MAX_NAME_LEN], struct afs_remote_lookup_result *res);

struct afs_remote_readdir_result {
	int handle;
	unsigned char ftype;
	char name[MAX_NAME_LEN];
};

int afs_remote_readdir(int dir_handle, struct afs_remote_readdir_result res[MAX_FILES_PER_READDIR]);

int afs_remote_unlink(int dir_handle, const char name[MAX_NAME_LEN]);

int afs_remote_mkdir(int dir_handle, const char name[MAX_NAME_LEN]);
int afs_remote_rmdir(int dir_handle, const char name[MAX_NAME_LEN]);

#endif /* _AFS_CLIENT_H_ */
