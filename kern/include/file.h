#ifndef _FILE_H_
#define _FILE_H_

struct open_file {
	off_t offset;
	int fd;
	struct vnode* file_ptr;
};

int fd = 0;


#endif /* _FILE_H_ */
