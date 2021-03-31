# File_system_Linux
myalloc_p.h : In header file there is code for malloc() function. 
formula.h : In formula.h file there is code for creating efficient block system for file system.
babystep2_p.c : This is the main code where i fetch/add those header files for reference. This code is totally written in 'C' language.
                
                1) fileInfo structure created for storing metadata of file. Like filename, startno, sizeinblocks, endbytes, fileno, nextblock.
                
                2) readDiskBlock(int fd,int blkno, void * buffptr) : This function is for reading block of disk. The parameter given to this function is 
                    fd:filediscriptor, blkno:block number which you want to access, *buffptr: It is the buffer in which we store the data of accessed file from 
                    that blocks.
                
                3) writeDiskBlock(int fd,int blkno,void * buffptr) : This function is for writing data onto the given blocks. 
                
                4) createDisk() : createDisk() is for creating disk of 100 MB size of file named with 'disk.teasage'. It is the static memory allocated from 
                    main memory. As i created virtual file system, all operation is happen in that statically allocated memory. The operation can be dynamic but
                    for that i created static memory.
                
                5) int vdcpto(char* filepath,char* filename): vdcpto is the function which finds the empty block for writing/saving data into memory. After finding 
                    memory it will put data into that blocks.
                
                6) int vdls(char*buf) : This function is same as ls command in linux system. vdls() shows user which files are stored in that disk with its name.
                
                7) int delete(char *filename) : delete() is used for delete the file data from memory. Actually we don't delete all file data, we only changes it's 
                    header and clean it's inode information. When we again store any files for that position then it will available for user.
                
                8) int vdcpfrom(char* filepath,char* filename) : vdcpfrom() is for copying all bytes from one memory and write it to virtual memory. 
                
                9) int main() : main() shows us all variety of operation that we want to apply on files. 
                
                
                It is the small/virtual file system. But I worked on Linux file sytem that is ext4 file sytem. 
                Reference : The Design Of UNIX Operating System by Maurice J Bach. Refer this book it will helpful for further developement also. 
                      
