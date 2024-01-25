#include "afs_fs.h"

#include "afs_inode.h"

int afs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *inode;
	inode = afs_get_inode(sb, NULL, S_IFDIR, 1000);
	sb->s_root = d_make_root(inode);
	if (sb->s_root == NULL) {
		return -ENOMEM;
	}
	pr_info("Superblock filled\n");
	return 0;
}

struct dentry *afs_mount(struct file_system_type *fs_type, int flags, const char *token, void *data)
{
	struct dentry *ret;

	ret = mount_nodev(fs_type, flags, data, afs_fill_super);
	if (ret == NULL) {
		pr_err("Failed to mount file system\n");
	} else {
		pr_info("Mounted successfuly\n");
	}

	return ret;
}

void afs_kill_sb(struct super_block *sb)
{
	pr_info("AFS super block is destroyed. Unmount successfully.\n");
}

static struct file_system_type afs_fs_type = { .name = "afs", .mount = afs_mount, .kill_sb = afs_kill_sb };

int afs_init_fs()
{
	int res = register_filesystem(&afs_fs_type);
	if (res != 0) {
		pr_err("Failed to register AFS\n");
		return res;
	}

	pr_info("AFS initialized\n");
	return 0;
}

void afs_exit_fs()
{
	int res = unregister_filesystem(&afs_fs_type);
	if (res != 0) {
		pr_err("Failed to unregister AFS\n");
	} else {
		pr_info("AFS successfully deinitialized\n");
	}
}