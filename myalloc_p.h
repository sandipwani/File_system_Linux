//It contains own implementation of malloc and free 
#include<stdio.h>
unsigned char buffer[200000];
struct Header{                      
        long unsigned int size;  //free size availale in this block
        int isfree;              //if isfree==1 we will consider this block as free and isfree==0 it will be occupied
        struct Header*next;      //point to next block
};
struct Header* freeL=(void*)buffer; 
void *myalloc(long unsigned int nbytes);
void myfree(void *freeptr);
void *myalloc(long unsigned int nbytes)
{
	struct Header *current,*previous;
	void*required;

	if(!(freeL->size))//initially
	{
		freeL->size=200000-sizeof(struct Header);
		freeL->isfree=1;
		freeL->next=NULL;
	}
	current=freeL;
	while((((current->size)<nbytes)||((current->isfree)==0))&&(current->next!=NULL))//if not required free block move next
	{
		current=current->next;

	}
	if(current->size==nbytes)//exact size is available
	{
		current->isfree=0;
		required=(void*)(++current);
		printf("\n\tblock of exact memory found");
	       	return required;	//since at current we have header and from current++ will have free space
	}
	else if((current->size)>(nbytes+sizeof(struct Header)))
	{
		//split the block into two blocks one with required size and another with remaining size

		struct Header*split2=(void*)((void*)current+nbytes+sizeof(struct Header));
		split2->size=current->size-nbytes-sizeof(struct Header);
		//printf("\nsplit2->size=%lu",split2->size);
		split2->isfree=1;          //can be use later
		split2->next=current->next;
		current->size=nbytes;
		//printf("\ncur->size=%lu",current->size);
		current->isfree=0;        //currently allocated
		current->next=split2;
		required=(void*)(++current);
		//printf("\nAllocation via splitting Done!");
		return required;
	}
	else
	{
                //No space available
		required=NULL;
		printf("\nNo space");
		return required;
	}
}
void myfree(void* freeptr)
{
	if(((void*)buffer<=freeptr) && (freeptr<=(void*)(buffer+200000)))//valid address
	{
		struct Header*current=freeptr;
		--current;  //since freeptr points to space beyond header we have to go back to make this block free;
		current->isfree=1;  //freeing the block
		struct Header *tempcur;
		tempcur=freeL;               //Traverse the list if two neighbouring blocks free merged them into one 
		while(tempcur->next!=NULL)
		{
			if(tempcur->isfree && tempcur->next->isfree)
			{
				tempcur->size+=(tempcur->next->size)+sizeof(struct Header);
				if(tempcur->next->next)
					tempcur->next=tempcur->next->next;
				else
					tempcur->next=NULL;

			}
			else
			{
				tempcur=tempcur->next;
			}
		}
	}
	//else
		//printf("\nNot valid address");
}

