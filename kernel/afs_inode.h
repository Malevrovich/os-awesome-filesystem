#ifndef _AFS_INODE_H_
#define _AFS_INODE_H_

#include <linux/fs.h>

struct inode *afs_get_inode(struct super_block *sb, const struct inode *dir,
			    umode_t mode, int i_ino);

#endif /* _AFS_INODE_H_ */