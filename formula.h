#include<stdio.h>
#define blocksize 512
#define total_struct 145
int head_struct = 8,data_struct = 1,free1;
unsigned long mult,mult1;
unsigned long disksize;
unsigned long blocks;
unsigned long header=16,address_pointer=1,data=128,temp_head,temp_add,temp_data;
int *temp;

int *formula(unsigned long disksize)
{
	blocks = disksize/blocksize;
	if(blocks<total_struct)
	{
		temp_add = 1;
		blocks = blocks - 1;
		mult1 = blocks/head_struct;
		if(blocks<head_struct)
		{
			temp_head = 1;
			blocks = blocks - 1;
			temp_data = blocks;
		}
		else
		{
			temp_head = mult1;
			blocks = blocks - mult1;
			temp_data = blocks;
		}
	}
	else
	{
		mult = blocks/total_struct;
		temp_head = mult*header;
		temp_add = mult*address_pointer;
		temp_data = mult*data;
		free1 = blocks-(mult*total_struct);
		if(free1 >= 3 && free1 <= total_struct)
		{
			temp_add = temp_add + 1;
			free1 = free1 - 1;
			mult = free1/head_struct*2;
			if(mult>=header)
			{
				mult = header;
			}
			if(free1<head_struct)
			{
				temp_head = temp_head + 1;
				free1 = free1 - 1;
				temp_data = temp_data + free1;
			}
			else
			{
				temp_head = temp_head + mult;
				free1 = free1 - mult;
				temp_data = temp_data + free1;
			}
		}
	}
	temp = (int *)myalloc(sizeof(int)*3);
	//printf("header of disk : %ld\n",temp_head);
	temp[0] = temp_head;
        //printf("address of disk : %ld\n",temp_add);
	temp[1] = temp_add;
        //printf("data of disk : %ld\n",temp_data);
	temp[2] = temp_data;
	return temp;
}

