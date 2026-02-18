#include "vfs.h"
#include "../memory/heap.h"
#include "../memory/string.h"

#define MAX_MOUNTPOINTS	12
mountpoint_t *mountpoints_root;

mountpoint_t *create_mountpoint(char *device, char *type, char *target)
{	
	mountpoint_t *new_mount = kalloc(sizeof(mountpoint_t));
	new_mount.device = device;
	new_mount.type = type;
	new_mount.mountpoint = target;
	new_mount.operations = NULL;

	return new_mount;
}

int vfs_mount(char *device, char *target, char *fs_type) 
{
	mountpoint_t *new_mount = create_mountpoint(device, type, target);

	mountpoint_t *current = mountpoints_root;
	
	if (!current) {
		mountpoints_root = new_mount;
		return 0;
	}

	do {
		current = current->next; 
	} while (current->next)

	if (!current)
		return 2;

	current->next = new_mount;
	return 0;
}

int vfs_umount(char *device, char *target)
{
	mountpoint_t *current = mountpoints_root;

	while (current) {
		if (!strcmp(current->device, device, strlen(device))) {
			current = current->next;
			continue;
		}
		if (!strcmp(current->target, target, strlen(target))) {
			current = current->next;
			continue;
		}

		// All are the same
		break;
	}

	if (!current)
		return 1;

	kfree(current);
	logf("Freed vfs mount!\n");
}

mountpoint_t *get_mountpoint(char *path)
{
	mountpoint_t *current = mountpoints_root;	

	while (current) {
		if (strcmp(current->mountpoint, path, strlen(current->mountpoint))) {
			break;
		}
		current = current->next;
	}

	return current;
}
