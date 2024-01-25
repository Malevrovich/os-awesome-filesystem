#include "afs_inode.h"

#include "afs_client.h"

static const struct inode_operations afs_inode_ops;
static const struct file_operations afs_dir_ops;

struct inode *afs_get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, int i_ino)
{
	struct inode *inode;

	inode = new_inode(sb);
	if (inode != NULL) {
		inode->i_ino = i_ino;
		inode->i_op = &afs_inode_ops;
		if (mode & S_IFDIR) {
			inode->i_fop = &afs_dir_ops;
		}
		inode_init_owner(sb->s_user_ns, inode, dir, mode | S_IRWXUGO);
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
	pr_info("Call lookup\n");

	ret = afs_remote_lookup(root, name, &res);
	if (ret == 0) {
		return NULL;
	}

	inode = afs_get_inode(parent_inode->i_sb, NULL, res.umode, res.handle);

	d_add(child_dentry, inode);

	pr_info("Lookup ok\n");
	return child_dentry;
	// return NULL;
}

int afs_iterate(struct file *filp, struct dir_context *ctx)
{
	int readdir_res_size;
	struct afs_remote_readdir_result readdir_res[MAX_FILES_PER_READDIR];
	char fsname[MAX_NAME_LEN];
	struct dentry *dentry;
	struct inode *inode;
	unsigned long offset;
	unsigned char ftype;
	int stored;
	ino_t ino;
	ino_t dino;
	pr_info("Call iterate\n");

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
			pr_info("Iterate ok\n");
			return stored;
		}
		dir_emit(ctx, fsname, strlen(fsname), dino, ftype);
		stored++;
		offset++;
		ctx->pos = offset;
	}
	return stored;
}

int afs_create(struct user_namespace *namespace, struct inode *parent_inode, struct dentry *child_dentry, umode_t mode,
	       bool b)
{
	ino_t root;
	struct inode *inode;
	const char *name = child_dentry->d_name.name;
	ino_t new_ino;
	root = parent_inode->i_ino;

	pr_info("Creating file\n");
	new_ino = afs_remote_create(root, name);
	if (new_ino == 0) {
		return -1;
	}

	inode = afs_get_inode(parent_inode->i_sb, NULL, S_IFREG | S_IRWXUGO, new_ino);
	d_add(child_dentry, inode);

	return 0;
}

static const struct inode_operations afs_inode_ops = {
	.lookup = afs_lookup,
	.create = afs_create,
};

static const struct file_operations afs_dir_ops = {
	.iterate = afs_iterate,
};