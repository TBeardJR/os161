#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_open(char *path, int oflag, int *ret_fd);
int sys_read(int fd, void *buf, size_t nbytes, int *ret_fd);
int sys_write(int fd, const void *buf, size_t nbytes, int *ret_fd);


#endif /* _SYSCALL_H_ */
