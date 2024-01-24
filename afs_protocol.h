#ifndef _AFS_PROTOCOL_H_
#define _AFS_PROTOCOL_H_

#include "afs_config.h"

typedef unsigned int fhandle; // in current implementation is inode->i_ino

enum AFS_REQUEST_TYPE {
	AFS_CREATE = 0,
	AFS_UNLINK,
	AFS_LOOKUP,
	AFS_READDIR,
	AFS_READ,
	AFS_WRITE
};

struct afs_dir_op_args {
	fhandle dir;
	char name[MAX_NAME_LEN];
} __attribute__((packed));

struct afs_readdir_args {
	fhandle dir;
} __attribute__((packed));

struct afs_read_op_args {
	fhandle dir;
	unsigned int offset;
} __attribute__((packed));

struct afs_write_op_args {
	fhandle dir;
	unsigned int offset;
	char data[IO_BUFFER_SIZE];
} __attribute__((packed));

struct afs_request {
	enum AFS_REQUEST_TYPE type;
	union {
		struct afs_dir_op_args as_create;
		struct afs_dir_op_args as_unlink;
		struct afs_dir_op_args as_lookup;
		struct afs_readdir_args as_readdir;
		struct afs_read_op_args as_read;
		struct afs_write_op_args as_write;
	} args;
} __attribute__((packed));

enum AFS_RESPONSE_STATUS { AFS_OK = 0, AFS_ERROR };

enum AFS_FILE_TYPE { AFS_FILE, AFS_DIR };

struct afs_lookup_result {
	fhandle handle;
	enum AFS_FILE_TYPE type;
	char name[MAX_NAME_LEN];
} __attribute__((packed));

struct afs_read_result {
	unsigned int size;
	char data[IO_BUFFER_SIZE];
} __attribute__((packed));

struct afs_response {
	enum AFS_RESPONSE_STATUS status;
	union {
		fhandle as_create;
		afs_lookup_result as_lookup;
		afs_lookup_result
			as_readdir[MAX_FILES_PER_READDIR]; // null terminated array
		fhandle as_read;
	} body;
} __attribute__((packed));

#endif /* _AFS_PROTOCOL_H_ */