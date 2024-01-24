#include <linux/init.h>
#include <linux/module.h>

#include "afs_fs.h"

static int __init afs_init(void)
{
	int res = afs_init_fs();
	if (res != 0) {
		pr_err("Failed to initialize AFS\n");
		return res;
	}

	pr_info("Successfully loaded\n");
	return 0;
}

static void __exit afs_exit(void)
{
	afs_exit_fs();
	pr_info("AFS unloaded\n");
}

module_init(afs_init);
module_exit(afs_exit);

MODULE_LICENSE("GPL");