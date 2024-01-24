#include "afs_inode.h"

static const struct inode_operations afs_inode_ops;

struct inode *afs_get_inode(struct super_block *sb, const struct inode *dir,
			    umode_t mode, int i_ino)
{
	struct inode *inode;
	inode = new_inode(sb);
	if (inode != NULL) {
		inode->i_ino = i_ino;
		inode->i_op = &afs_inode_ops;
		inode_init_owner(sb->s_user_ns, inode, dir, mode);
	}
	return inode;
}

struct dentry *afs_lookup(struct inode *parent_inode,
			  struct dentry *child_dentry, unsigned int flag)
{
	return NULL;
}

afs_inode_ops = (struct inode_operations){
	.lookup = afs_lookup,
};