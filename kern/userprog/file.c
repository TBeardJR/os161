#include <types.h>
#include <syscall.h>
#include <uio.h>
#include <lib.h>
#include <kern/errno.h>
#include <thread.h>
#include <curthread.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>

/* implementation of sys_open() */
int
sys_open(char *path, int oflag, int *ret_fd)
{
	struct open_file openedFile;
	int result = vfs_open(&path, oflag, &openedFile.file_ptr);
	if(result) {
		return result;
	}
	openedFile.offset = 0;
	curthread->open_file_table[fd] = openedFile;
	*ret_fd = fd;
	fd++;
	return 0;
}

int
sys_read(int fd, void *buf, size_t nbytes, int *ret_fd)
{
	struct uio my_uio;
	struct open_file openedFile = curthread->open_file_table[fd];
	mk_kuio(&my_uio, &buf, nbytes, openedFile.offset, UIO_READ);
	int result = VOP_READ(openedFile.file_ptr, &my_uio);
	if(result) {
		kprintf("FD (%d): Read error: %s\n", fd, strerror(result));
		vfs_close(openedFile.file_ptr);
		return result;
	}

	if (my_uio.uio_resid != 0) {
		/* short read; problem with file? */
		kprintf("ELF: short read on FD: %d - file truncated?\n", fd);
		vfs_close(openedFile.file_ptr);
		return ENOEXEC;
	}
	openedFile.offset = my_uio.uio_offset;
	*ret_fd = openedFile.offset;
	return 0;	
}

int
sys_write(int fd, const void *buf, size_t nbytes, int *ret_fd)
{
	struct uio my_uio;
	struct open_file openedFile = curthread->open_file_table[fd];
	mk_kuio(&my_uio, &buf, nbytes, openedFile.offset, UIO_WRITE);
	int result = VOP_WRITE(openedFile.file_ptr, &my_uio);
	if(result) {
		kprintf("FD (%d): Write error: %s\n", fd, strerror(result));
		vfs_close(openedFile.file_ptr);
		return result;
	}

	if (my_uio.uio_resid > 0) {
		/* short write; problem with file? */
		kprintf("ELF: short write on FD: %d - file truncated?\n", fd);
		vfs_close(openedFile.file_ptr);
		return ENOEXEC;
	}
	openedFile.offset = my_uio.uio_offset;
	*ret_fd = openedFile.offset;
	return 0;	
}


 