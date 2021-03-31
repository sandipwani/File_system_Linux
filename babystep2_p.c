#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<errno.h>
#include<string.h>
#include"myalloc_p.h"
#include"formula.h"

struct fileInfo
{
        char filename[44];
        int startno;  //starting block number of file.
        int sizeinblocks;     //file size in blocks
        int endbytes; //available bytes in the end block of file
	int fileno;     //for how many files in disk
	int nextblock;  //for storing next linked block to access
};
struct linkInfo
{
	int link;
};

char nullbyte[] = "\0";

int readDiskBlock(int fd,int blkno, void * buffptr)
{
	if(lseek(fd,blkno*512,SEEK_SET)==-1)
	{
		return -1;
	}
	return read(fd,buffptr,512);
}

int writeDiskBlock(int fd,int blkno,void * buffptr)
{
	if(lseek(fd,blkno*512,SEEK_SET)==-1)
	{
		return -1;
	}
	return (write(fd,buffptr,512));
}

int createDisk()
{
    off_t disksize = 104857600L-1L;   //Here we create disk with size 100 MB
    int *arr=(int*)myalloc(sizeof(int)*3);
    arr = formula(disksize);		//formula() is stored in formula.h file where i could divide disk into 3 appropriate parts for better seeking to file in future 
    int fd = open("disk.teasage", O_RDWR | O_CREAT, 00700);
    if (fd ==-1) //some error
    {
      	return -1;
    }
    if (write(fd, nullbyte, 1) != 1)
    {
       	return -2;
    }
    if (lseek(fd, disksize, SEEK_SET) == -1)
    {
	return -3;
    }

    if (write(fd, nullbyte, 1) != 1)
    {
	return -2;
    }
    for(int i=0;i<arr[0];i++)
    {
        struct fileInfo*look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
        for(int j=0;j<8;j++)
        {
                strcpy((look+j)->filename,"-1");
                look[j].startno=-1;
                look[j].sizeinblocks=-1;
                look[j].endbytes=-1;
                look[j].fileno=-1;
		look[j].nextblock=-1;
        }
        if(writeDiskBlock(fd,i,look)==-1)
	{
		return -3;
	}
        myfree(look);
    }
    if(lseek(fd,0,SEEK_SET)==-1)
    {
            return -3;
    }
    for(int i=arr[0];i<arr[1];i++)
    {
	    struct linkInfo *look=(struct linkInfo*)myalloc(128*sizeof(struct linkInfo));
	    for(int j=0;j<128;j++)
	    {
		    look[j].link = -1;
	    }
	    if(writeDiskBlock(fd,i,look)==-1)
	    {
		    return -3;
	    }
	    myfree(look);
    }
    if(lseek(fd,0,SEEK_SET)==-1)
    {
	    return -3;
    }
    close(fd);
    int temp = arr[0]+arr[1]+arr[2]+1;
    return temp;
}

int checkfilledsize()
{
	int f_size=0;
	int vdfd=open("disk.teasage",O_RDONLY,00700);
	if(vdfd ==-1)
	{return -2;}
	struct fileInfo *look = (struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
	readDiskBlock(vdfd,0,look);
	if(look[0].fileno == -1)
	{return -5;}
	int blockend = (look[0].fileno)/8;
	int fileend = (look[0].fileno)%8;
	int k=0;
	if(blockend==0 && fileend !=7)
	{
		for(int start=0;start<=fileend;start++)
		{
			if(strcmp((look+start)->filename,"-1")==0 && (look+start)->fileno > -1)
			{
				f_size = f_size + (look+start)->sizeinblocks;
			}
		}
		myfree(look);
		close(vdfd);
		return f_size;
	}

	for(int i=0;i<blockend;i++)
	{
		look = (struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
		readDiskBlock(vdfd,i,look);
		for(int j=0;j<8;j++)
		{
			if(strcmp((look+j)->filename,"-1")==0 && (look+j)->fileno > -1)
                        {
                                f_size = f_size + (look+j)->sizeinblocks;
                        }
		}
		myfree(look);
	}
	look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
	readDiskBlock(vdfd,blockend,look);
	for(int i=0;i<=fileend;i++)
	{
		if(strcmp((look+i)->filename,"-1")==0 && (look+i)->fileno > -1)
                {
                        f_size = f_size + (look+i)->sizeinblocks;
                }
	}
	myfree(look);
	close(vdfd);
	return f_size;
}

int vdcpto(char* filepath,char* filename)
{
	off_t disksize = 104857600L-1L;
        int *arr=(int*)myalloc(sizeof(int)*3);
	arr = formula(disksize);
	int vdstartblock=arr[2]; 		//files writing starts from block arr[2] having a reserved block value of data section
	int vdblock=vdstartblock;		//vdblock starts also start from as above
	int vd_block=vdblock;
	int readbytes,endbytes,filesize,filesizeinblocks;
	char buf[512];
	int vdfd=open("disk.teasage",O_RDWR,00700);  //open vdfd in Readwrite mode
	if(vdfd==-1)
	{
		return -2;
	}
	int adfd=open(filepath,O_RDONLY,00700);
	if(adfd==-1)
	{
		return -3;
	}
	if((filesize=lseek(adfd,0,SEEK_END))==-1)//Calculate filesize
	{
		return -4;
	}
	filesizeinblocks=filesize/512; //filesize in blocks
	if(filesize%512)
		filesizeinblocks++;  //if extra bytes then one block increases
        
	struct fileInfo*look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));  //fileInfo structure
	if(readDiskBlock(vdfd,0,look)==-1)
	{                                           //read block 0 and stored in buffer look;
		return -1;
	}
	close(adfd);   //close adfd

	adfd=open(filepath,O_RDONLY,00700);//open adfd
	if(adfd==-1)
	{
		return -3;
	}
        if(lseek(adfd,0,SEEK_SET)==-1)     //at the start of actual disk for reading
	{
		return -4;
	}
	if(look[0].fileno==-1) // Disk is Empty, no files in the disk
	{
		if(filesizeinblocks > arr[2]) //if filesizeinblocks > available blocks 
		{
			//printf("\nNo Disk Space");
			return -5;
		}
                
		readbytes=read(adfd,buf,512);     //read first block of actual  disk
		while(readbytes!=0)
		{
			if(writeDiskBlock(vdfd,vdblock,buf)==-1)//write in vd at vdblock no. first time it is 2275;
			{
				return -1;
			}
			vdblock++;
			endbytes=readbytes;
			if((readbytes=read(adfd,buf,512))==-1)
			{
				close(adfd);
				close(vdfd);
				return -6;
			}
		}
	
		struct fileInfo* block=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));// block fileInfo pointer
		readDiskBlock(vdfd,0,block);
		block[0].startno=arr[0]+arr[1]+1;                     //writing into first block header
		block[0].sizeinblocks=filesizeinblocks;
		block[0].endbytes=endbytes;
		block[0].fileno=0;
		block[0].nextblock=-1;
		strcpy((block+0)->filename,filename);
		writeDiskBlock(vdfd,0,block);            
		myfree(block);
	}
	else
	{
		/*int fill_block = check_fill_block_vd();
		int free_block = 18205-fill_block;
		if(free_block == 0 && free_block < filesizeinblocks)
		{
			return -5;
		}*/
		int block_no=(look[0].fileno)/8;    //calculating in which block no. I have to see for the position of new file
		int file_no=(look[0].fileno)%8;     //calculating which file no in that block no
		if(readDiskBlock(vdfd,block_no,look)==-1)
		{
			return -1;
		}
		int last_unfill = arr[2]-look[file_no].startno+look[file_no].sizeinblocks;
		int total_unfill = last_unfill + checkfilledsize();
		//vdblock=look[file_no].startno+look[file_no].sizeinblocks; //point to block_no where to copy new file
		//vd_block=vdblock;
		if(total_unfill < filesizeinblocks)
		{
			perror("NO Space in the disk");
			return -5;
		}
		//check if memory is stored in contineous or scattered manner.
		if(checkfilledsize() == 0)
		{
			endbytes=readbytes=read(adfd,buf,512);
			while(readbytes!=0)
			{
				if(writeDiskBlock(vdfd,vdblock,buf)==-1)
				{
					return -1;
				}
				vdblock++;
				endbytes=readbytes;
				if((readbytes=read(adfd,buf,512))==-1)
				{
					close(adfd);
					close(vdfd);
					return -6;
				}
			}
			
	        	if(readDiskBlock(vdfd,0,look)==-1)
			{
				return -1;
			}
			look[0].fileno+=1;
			if(writeDiskBlock(vdfd,0,look)==-1)
			{
				return -1;
			}
			block_no=look[0].fileno/8;
	        	file_no=look[0].fileno%8;
			readDiskBlock(vdfd,block_no,look);
	        	look[file_no].startno=vd_block;
			look[file_no].endbytes=endbytes;
                	look[file_no].fileno=look[0].fileno;
                	look[file_no].sizeinblocks=filesizeinblocks;
			look[file_no].nextblock=-1;
                	strcpy((look+file_no)->filename,filename);
			if(writeDiskBlock(vdfd,block_no,look)==-1)
			{
				return -1;
			}	
		}
		else 
		{
			// Look first empty block 
			int tempsize = filesizeinblocks;
			int skip=0,temp_block,temp_file;
			while(tempsize != 0)
			{
				for(int i=0;i<=(look[0].fileno)+1;i++)
				{
					if(strcmp((look+i)->filename,"-1")==0 && (look+i)->fileno > -1)
					{
						/*if(skip > 0)
						{
							readDiskBlock(vdfd,temp_block,look);
							look[temp_file].nextblock = look[i].startno;					
						}*/
						endbytes=readbytes=read(adfd,buf,512);
						int startno1 = look[i].startno;
						int filesize1 = look[i].sizeinblocks;
						while(readbytes!=0 && startno1 < filesize1)
                        			{
                                			if(writeDiskBlock(vdfd,startno1,buf)==-1)
                                			{
                                        			return -1;
                                			}
                                			startno1++;
							tempsize--;
                                			endbytes=readbytes;
                                			if((readbytes=read(adfd,buf,512))==-1)
                                			{
                                			        close(adfd);
                                			        close(vdfd);
                                			        return -6;
                                			}
                        			}
			                        if(readDiskBlock(vdfd,0,look)==-1)
			                        {
                        				return -1;
                        			}
                        			look[0].fileno+=1;
                        			if(writeDiskBlock(vdfd,0,look)==-1)
                        			{
                        			        return -1;
                        			}
                        			temp_block = block_no=look[i].fileno/8;
                        			temp_file = file_no=look[i].fileno%8;
                        			readDiskBlock(vdfd,block_no,look);
                        			look[file_no].startno=vd_block;
                        			look[file_no].endbytes=endbytes;
                        			look[file_no].fileno=look[0].fileno;
                        			look[file_no].sizeinblocks=filesizeinblocks;
                        			//look[file_no].nextblock=-1;
                        			strcpy((look+file_no)->filename,filename);
					        for(int i=0;i<block_no;i++)
					        {
        					        look = (struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
        					        readDiskBlock(vdfd,i,look);
                					for(int j=0;j<8;j++)
               			 			{
                        					if(strcmp((look+j)->filename,"-1")==0 && (look+j)->fileno > -1)
                        					{
                        					        //f_size = f_size + (look+j)->sizeinblocks;
                        					}
                					}
                					myfree(look);
        					}
                        			if(writeDiskBlock(vdfd,block_no,look)==-1)
                        			{
                                			return -1;
                        			}
					}
					else
					{
						skip++;
					}
				}
			}
		}
	}

	close(adfd);
	close(vdfd);
	myfree(look);
	return 0;
}

int vdls(char*buf)
{
	int vdfd=open("disk.teasage",O_RDONLY);
	if(vdfd==-1)
	{
		return -2;
	}
	struct fileInfo* look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
	readDiskBlock(vdfd,0,look);
	if(look[0].fileno==-1)
	{
		return -5;
	}
	int blockend=(look[0].fileno)/8;
	int fileend=(look[0].fileno)%8;
	int k=0;

	if(blockend==0 && fileend!=7)
	{
		for(int start=0;start<=fileend;start++)
		{
			if(strcmp(((look+start)->filename),"-1") == 0)
			{
				//buf = buf;
				//k++;
				/*int t_start = start;
				t_start++;
				while(strcmp((look+t_start)->filename,"-1")==0)
				{
					buf = buf+48;
					t_start++;
				}*/
			}
			else
			{
				strcpy((buf+k*48),(look+start)->filename);
				k++;
			}
		}
		strcpy((buf+k*48),"-1");
		myfree(look);
		close(vdfd);
		return 0;
		
	}
	for(int i=0;i<blockend;i++)
	{
		look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
		readDiskBlock(vdfd,i,look);
		for(int j=0;j<8;j++)
		{     
                        if(strcmp(((look+j)->filename),"-1") == 0)
                        {
                                //buf = buf+k*48;
				//k++;
                                /*int t_start = j;
                                t_start++;
                                while(strcmp((look+t_start)->filename,"-1")==0)
                                {
                                        buf = buf+48;
                                        t_start++;
                                }*/
                        }
			else
			{
				strcpy((buf+k*48),(look+j)->filename);
				k++;
			}
		}
		myfree(look);
	}
	look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
       	readDiskBlock(vdfd,blockend,look);
	for(int i=0;i<=fileend;i++)
	{
                if(strcmp(((look+i)->filename),"-1") == 0)
                {
                       //buf = buf+k*48;
		       //k++;
                       /*int t_start = i;
                       t_start++;
                       while(strcmp((look+t_start)->filename,"-1")==0)
                       {
                                buf = buf+48;
                                t_start++;
                       }*/
                }
		strcpy((buf+k*48),(look+i)->filename);
		k++;
	}
	
        strcpy((buf+k*48),"-1");
	myfree(look);
	close(vdfd);
	return 0;
}
int delete(char *filename)
{
	int vdfd=open("disk.teasage",O_RDWR,00700);
	if(vdfd==-1)
	{
		return -2;
	}
	char buf[512];
	struct fileInfo*look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
	readDiskBlock(vdfd,0,look);
	int totalfiles = look[0].fileno;
	if(totalfiles==-1)
	{
		return -5;
	}
	int blockend=totalfiles/8;
	myfree(look);
	for(int start=0;start<=blockend;start++)
	{
		struct fileInfo *look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
		readDiskBlock(vdfd,start,look);
		for(int blockfile=0;blockfile<8;blockfile++)
		{
			if(strcmp((look+blockfile)->filename,filename)==0)
			{
				struct fileInfo *block=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
      				readDiskBlock(vdfd,0,block);
				block[0].fileno = (block[0].fileno)-1;
				//myfree(look);
				strcpy((block+blockfile)->filename,"-1");  //here we changed file name as "-1"
				writeDiskBlock(vdfd,0,block);
                		//look[blockfile].startno=-1;
                		//look[blockfile].sizeinblocks=-1;
                		//look[blockfile].endbytes=-1;
		                //look[blockfile].fileno=-1;
				close(vdfd);
				myfree(block);
                                return 0;
			}
		}
		//writeDiskBlock(vdfd,start,look);
		//close(vdfd);
		myfree(look);
		//return 0;
	}
	close(vdfd);
        return -4;
}
int vdcpfrom(char* filepath,char* filename)
{
	int vdfd=open("disk.teasage",O_RDONLY,00700);
	if(vdfd==-1)
	{
		return -2;
	}
	char buf[512];
	struct fileInfo*look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
	readDiskBlock(vdfd,0,look);
	int totalfiles=look[0].fileno;
	if(totalfiles==-1)
	{
		return -5;
	}
	int blockend=totalfiles/8;
	myfree(look);
	for(int start=0;start<=blockend;start++)
	{
		look=(struct fileInfo*)myalloc(8*sizeof(struct fileInfo));
		readDiskBlock(vdfd,start,look);
		for(int blockfile=0;blockfile<8;blockfile++)
		{
			if(strcmp((look+blockfile)->filename,filename)==0)  //file found
			{
				//strcat(filepath,"/");
				//strcat(filepath,filename);
				int fileCopy=open(filepath, O_WRONLY | O_CREAT,00700);
				if(fileCopy==-1)
				{
					return -3;
				}
				int startblock=look[blockfile].startno;
				while(startblock<(look[blockfile].startno+look[blockfile].sizeinblocks-1))
				{
					readDiskBlock(vdfd,startblock,buf);
					write(fileCopy,buf,512);
					startblock++;
				}
				readDiskBlock(vdfd,startblock,buf);
				write(fileCopy,buf,look[blockfile].endbytes);
				close(fileCopy);
				close(vdfd);
				myfree(look);
				return 0;
			}
		}
		myfree(look);
	}
	
	close(vdfd);
	return -4;

}


int main()
{
	off_t disksize = 104857600L-1L;
	//int disksize;
	int *arr=(int*)myalloc(sizeof(int)*3);
	arr = formula(disksize);
	/*for(int i=0;i<3;i++)
	{
		printf("%d\t",arr[i]);
	}
	printf("\n\n");
  	//return 0;*/
    	int errnum= createDisk();
    	if(errnum==-1)
    	{
		printf("\n Cannot create the virtual Disk");
		perror("\n Open error");
		exit(1);
    	}
    	if(errnum==-2)
    	{
		perror("\n Write Error while creating Virtual Disk");
		exit(1);
    	}
    	if(errnum==-3)
    	{
		perror("\n lseek error in VD");
		exit(1);
    	}
	float mb = errnum/2/1024;
    	printf("\nCongratulations!\n %f is your brand new harddisk is created fresh just now!! Enjoy!!!\n\n",mb);

        int ch,i;
        char filepath[100],filename[48];
	long max_block = arr[2];
	char buf[18275][48];

        do
        {
                printf("\n*********Menu*********\n");
                printf("\n1.Copy file from Actual Disk to Virtual Disk\n2.Display all files\n3.Copy file from virtual disk to actual\n4.Delete file from disk\n5.Exit");
                printf("\nEnter your Choice: ");
                scanf("%d",&ch);
                switch(ch)
                {
			case 1: printf("\nEnter Filepath which file you want to copy : ");
                                scanf(" %s",filepath);
				printf("\nEnter filename which you want to give to new file in VD : ");
				scanf(" %s",filename);
				errnum=vdcpto(filepath,filename);
				if(errnum==-1)
				{
					perror("\n lseek error in the virtual disk");
					exit(0);
				}
				if(errnum==-2)
				{
					perror("\n open error in the virtual disk");
					exit(0);
				}
				if(errnum==-3)
				{
					perror("\n open error for the file in Actual disk");
                                        exit(0);
				}
				if(errnum==-4)
				{
					perror("\n lseek error in the Actual Disk");
				        exit(0);
				}
				if(errnum==-5)
				{
					perror("\n NO space remaining in VD");
					exit(0);
				}
				if(errnum==-6)
				{
					perror("\n Read error for file in AD");
				        exit(0);
				}
					

                                break;
			case 2: //printf("Files Present: \n");
				errnum=vdls(&buf[0][0]);
				if(errnum==-2)
                                {
                                        perror("\n open error in the virtual disk");
                                        exit(0);
                                }
				if(errnum==-5)
				{
					perror("\n No files in the vd");
					break;
				}
				printf("Files Present : \n");	
				for(i=0;i<arr[2];i++)
				{
                                    if((strcmp(buf[i],"-1"))==0)
					    break;
				    printf("\t%s\n",buf[i]);
				}
                                
                  		break;
			case 3: 
				printf("\nEnter filename which you want to COPY: ");
				scanf(" %s",filename);
				printf("\nEnter filepath where you want to COPY: ");
                                scanf(" %s",filepath);
				errnum=vdcpfrom(filepath,filename);
				if(errnum==-2)
                                {
                                        perror("\n open error in the virtual disk");
                                        exit(0);
                                }
				if(errnum==-5)
                                {
                                        perror("\n No files in the vd");
                                }
				if(errnum==-3)
				{
					perror("\n Error opening at given filepath");
				}
				if(errnum==-4)
				{
					perror("\n file not present in the disk");
				}

				break;

			case 4:
				printf("\nEnter which file you want to delete from disk : ");
				scanf("%s",filename);
				errnum=delete(filename);
				if(errnum == -2)
				{
					perror("\n open error in the virtual disk");
					exit(0);
				}
				if(errnum == -5)
				{
					perror("\n No files in the vd");
				}
				if(errnum==-4)
                                {
                                        perror("\n file not present in the disk");
                                }
				break;

			case 5: exit(0);
                                break;
                        default:
                                printf("Enter valid Choice\n");
                }
        }while(ch!=0);
}

