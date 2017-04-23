/*
 * File-related system call implementations.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <copyinout.h>
#include <vfs.h>
#include <vnode.h>
#include <openfile.h>
#include <filetable.h>
#include <syscall.h>

/*
 * open() - get the path with copyinstr, then use openfile_open and
 * filetable_place to do the real work.
 */
int
sys_open(const_userptr_t upath, int flags, mode_t mode, int *retval)
{
	const int allflags = O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_NOCTTY;

	char kpath[256];
	struct openfile *file;
	int result = 0;
	size_t getlen;
	
	if (flags > allflags)
		return EDOM; //arguement out of range
	
	//int copyinstr(const_userptr_t usersrc, 
	//				char *dest, 
	//				size_t len, 
	//				size_t *got);
	//len??
	//got??
	result = copyinstr(upath, kpath, 256, &getlen);

	if(result) //error encountered
		return result;

	//open the file
	result = openfile_open(kpath, flags, mode, &file);

	if(result) //error encountered
		return result;


	//place file in curprocs filetable
	result = filetable_place(curproc->p_filetable, file, retval);

	if(result) //error encountered
		return result;

	return 0;


	/* 
	 * Your implementation of system call open starts here.  
	 *
	 * Check the design document design/filesyscall.txt for the steps
	 */
	//(void) upath; // suppress compilation warning until code gets written
	//(void) flags; // suppress compilation warning until code gets written
	//(void) mode; // suppress compilation warning until code gets written
	//(void) retval; // suppress compilation warning until code gets written
	//(void) allflags; // suppress compilation warning until code gets written
	//(void) kpath; // suppress compilation warning until code gets written
	//(void) file; // suppress compilation warning until code gets written

}

/*
 * read() - read data from a file
 */
int
sys_read(int fd, userptr_t buf, size_t size, int *retval)
{
	struct openfile* file; 
    int result = 0;

    //translate file descriptor number to open file object
    result = filetable_get(curproc->p_filetable, fd, &file);

    if(result) //error encountered
    	return result;
       
    //lock seek position in the openfile (but only for seekable objects)
    lock_acquire(file->of_offsetlock);

    //check for write-only
    if(file->of_accmode == O_WRONLY)
    	return EACCES;

    //cons up a uio??
    char* kbuf = (char*)buf;
    struct iovec useriov;
    struct uio userio;
    uio_kinit(&useriov, &userio, kbuf, size, file->of_offset, UIO_READ);


    //call VOP_READ
    result = VOP_READ(file->of_vnode, &userio);

    if(result)
    	return result;

    //update the seek position afterwards
    file->of_offset = userio.uio_offset;

    //unlock and filetable_put()
    lock_release(file->of_offsetlock);

    filetable_put(curproc->p_filetable, fd, file);

    //set correct return 
    *retval = 0;

    return 0;





    	/* 
        * Your implementation of system call read starts here.  
        *
        * Check the design document design/filesyscall.txt for the steps
        */
       //(void) fd; // suppress compilation warning until code gets written
       //(void) buf; // suppress compilation warning until code gets written
       //(void) size; // suppress compilation warning until code gets written
       //(void) retval; // suppress compilation warning until code gets written
}

/*
 * write() - write data to a file
 */
int
sys_write(int fd, userptr_t buf, size_t size, int *retval)
{

	//same for read except that it writes

	struct openfile* file; 
    int result = 0;

    //translate file descriptor number to open file object
    result = filetable_get(curproc->p_filetable, fd, &file);

    if(result) //error encountered
    	return result;
       
    //lock seek position in the openfile (but only for seekable objects)
    lock_acquire(file->of_offsetlock);

    //check for read-only
    if(file->of_accmode == O_RDONLY)
    	return EACCES;

    //cons up a uio??
    char* kbuf = (char*)buf;
    struct iovec useriov;
    struct uio userio;
    uio_kinit(&useriov, &userio, kbuf, size, file->of_offset, UIO_WRITE);


    //call VOP_WRITE
    result = VOP_WRITE(file->of_vnode, &userio);

    if(result)
    	return result;

    //update the seek position afterwards
    file->of_offset = userio.uio_offset;

    //unlock and filetable_put()
    lock_release(file->of_offsetlock);

    filetable_put(curproc->p_filetable, fd, file);

    //set correct return value
    *retval = 0;

    return 0;

}

/*
 * close() - remove from the file table.
 */
int
sys_close(int fd, int* retval)
{
	*retval = 0;
	struct openfile* oldfile;

	//validate the fd number (use filetable_okfd)
	if (!filetable_okfd(curproc->p_filetable, fd)) 
		return EBADF;

	//use filetable_placeat to replacecurprocs file table entry with NULL
	filetable_placeat(curproc->p_filetable, NULL, fd, &oldfile);

	//check if the previous entry in the filetable was also NULL
	//(this means no such file was open)
	if(curproc->p_filetable->ft_openfiles[fd-1] == NULL)
		return 1; //error thing happened maybe?

	if (oldfile == NULL)
		return ENOENT;

	//decref the open file returned by filetable_placeat
	openfile_decref(oldfile);

	return *retval;

}
/* 
* encrypt() - read and encrypt the data of a file
*/

/*
int
sys_encrypt(const_userptr_t upath, int* retval)
{
	int fd = 0;
	
	result = sys_open(upath, O_RDWR);


	sys_read(result)
}
*/
