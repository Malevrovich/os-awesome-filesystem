#include "afs_inode.h"

#include "afs_client.h"

static const struct inode_operations afs_inode_ops;
static const struct file_operations networkfs_dir_ops;

struct inode *afs_get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, int i_ino)
{
	struct inode *inode;
	inode = new_inode(sb);
	if (inode != NULL) {
		inode->i_ino = i_ino;
		inode->i_op = &afs_inode_ops;
		if (mode & S_IFDIR) {
			inode->i_fop = &networkfs_dir_ops;
		}
		inode_init_owner(sb->s_user_ns, inode, dir, mode);
	}
	return inode;
}

struct dentry *afs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flag)
{
	struct inode *inode;
	ino_t root = parent_inode->i_ino;
	const char *name = child_dentry->d_name.name;
	int ret;
	struct afs_remote_lookup_result res;

	ret = afs_remote_lookup(root, name, &res);
	if (ret) {
		return NULL;
	}

	inode = afs_get_inode(parent_inode->i_sb, NULL, res.umode, res.handle);

	d_add(child_dentry, inode);
	return child_dentry;
}

int afs_iterate(struct file *filp, struct dir_context *ctx)
{
	int readdir_res_size;
	struct afs_remote_readdir_result readdir_res[MAX_FILES_PER_READDIR];
	char fsname[256];
	struct dentry *dentry;
	struct inode *inode;
	unsigned long offset;
	unsigned char ftype;
	int stored;
	ino_t ino;
	ino_t dino;

	dentry = filp->f_path.dentry;
	inode = dentry->d_inode;
	offset = filp->f_pos;
	stored = 0;
	ino = inode->i_ino;

	readdir_res_size = afs_remote_readdir(ino, readdir_res);
	if (readdir_res_size < 0) {
		return -1;
	}

	while (true) {
		if (offset == 0) {
			strcpy(fsname, ".");
			ftype = DT_DIR;
			dino = ino;
		} else if (offset == 1) {
			strcpy(fsname, "..");
			ftype = DT_DIR;
			dino = dentry->d_parent->d_inode->i_ino;
		} else if (offset - 2 < readdir_res_size) {
			strcpy(fsname, readdir_res[offset - 2].name);
			ftype = readdir_res[offset - 2].ftype;
			dino = readdir_res[offset - 2].handle;
		} else {
			return stored;
		}
		dir_emit(ctx, fsname, readdir_res[offset - 2].name_len, dino, ftype);
		stored++;
		offset++;
		ctx->pos = offset;
	}

	return stored;
}

static const struct inode_operations afs_inode_ops = {
	.lookup = afs_lookup,
};

static const struct file_operations networkfs_dir_ops = {
	.iterate = afs_iterate,
};