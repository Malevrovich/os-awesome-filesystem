#ifndef _AFS_CLIENT_H_
#define _AFS_CLIENT_H_

#include <linux/fs.h>

#include "afs_config.h"

int afs_remote_create(int dir_handle, char name[MAX_NAME_LEN]);

#endif /* _AFS_CLIENT_H_ */
