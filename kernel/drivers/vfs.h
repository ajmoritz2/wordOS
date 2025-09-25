/*
 *	WORDOS VFS_H
 *	08/17/25
 *
 */
#ifndef		VFS_H
#define		VFS_H

#include <stdint.h>

#define VFS_TYPE_MAX	32
#define VFS_PATH_MAX	64

typedef struct vfs_operations {
	int (*open) (const char *path, int flags, ...);
	int (*close)(int file_descriptor);
	size_t (*read)(int file_descriptor, char *read_buffer, size_t nbyte);
	size_t (*write)(int file_descriptor, const void* write_buffer, size_t nbyte);
} vfs_operations;

typedef struct vfs_mountpoint{
	
	char device[VFS_TYPE_MAX];
	char type[VFS_TYPE_MAX];
	char mountpoint[VFS_TYPE_MAX];

	vfs_operations operations;

	struct vfs_mountpoint *next;

} mountpoint_t;

int vfs_mount(char *device, char *target, char fs_type);
int vfs_umount(char *device, char* target);
mountpoint_t *get_mountpoint(char *path)


#endif
