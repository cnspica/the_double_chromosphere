/***********************************
 *                                 *
 *      ^_^ Good Luck ^_^          *
 *                                 *
 *		  3null @ 2014.10.29       *
 ***********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include <assert.h>

#define	S_OK					(0x00)
#define	S_FAILED				(0x01)
#define	ONE_TICKET_NUMBERS	(0x07)
#define	MAX_SIZE				(0x06)
#pragma pack(1)

typedef enum
{
	RED_NUMBER,
	BLUE_NUMBER
}number_type_enum;

typedef struct
{
	unsigned long long seq_number;
	char ticket_number[ONE_TICKET_NUMBERS];
}ticket;


unsigned long long seq_number = 0;
ticket *global_ticket = NULL;
unsigned long long *choiced_seq_addr = NULL;
unsigned long long *choice_seq = NULL;


/*
 *random a number depend on ball type
 */
char random_one_number(number_type_enum type)
{
	char base = 0, result = 0;

	if(RED_NUMBER == type)
		base = 33;
	else if(BLUE_NUMBER == type)
		base = 16;
	else
		assert(0);

	result = 1 + (rand() % base);

	return result;	
}

char find_char_in_array(char finder, char *array, char size)
{
	char loop_idx = 0;
	
	assert(array);

	for(loop_idx = 0; loop_idx < size; loop_idx++){
		if(finder == *array++)
			return S_OK;
	}

	return S_FAILED;
}


/*数组排序，冒泡*/
void sort_array_bubble(char *array, char size)
{
	char idx_x = size, idx_y = 0, temp = 0;
	
	while(idx_x > 0)
	{
		for(idx_y = 0; idx_y < idx_x - 1; idx_y++)
		{
			if(array[idx_y] > array[idx_y + 1])
			{
				temp = array[idx_y];
				array[idx_y] = array[idx_y + 1];
				array[idx_y + 1] = temp;
			}
		}
		idx_x--;
	}
}


/*
 *根据类型，产生一组号码
 */
char random_a_set_of_number(number_type_enum type, char *result)
{
	char number_of_numbers = 0;
	char loop_idx = 0;
	char temp_array[MAX_SIZE], one_number = 0, save_idx  = 0;

	assert(result);

	memset(temp_array, 0, MAX_SIZE);
	
	if(RED_NUMBER == type)
		number_of_numbers = 6;
	else if(BLUE_NUMBER == type)
		number_of_numbers = 1;
	else
		assert(0);

	/*产生指定个数随机数*/
	for(loop_idx = 0; loop_idx < number_of_numbers; loop_idx++)
	{
		one_number = random_one_number(type);
		temp_array[save_idx++] = one_number;
		if(S_OK == find_char_in_array(one_number, temp_array, save_idx - 1))
		{
			save_idx--;
			temp_array[save_idx] = 0;
			loop_idx--;
			continue;
		}
	}

	/*排序*/
	if(RED_NUMBER == type)
	{
		sort_array_bubble(temp_array, number_of_numbers);
	}
	
	for(loop_idx = 0; loop_idx < number_of_numbers; loop_idx++)
	{
		*(result + loop_idx) = temp_array[loop_idx];
	}

	return number_of_numbers;
}


/*
 *产生一张票
 */
void random_a_ticket(ticket *ticket)
{
	char offset = 0;
	
	offset = random_a_set_of_number(RED_NUMBER, ticket->ticket_number);
	offset += random_a_set_of_number(BLUE_NUMBER, (char *)(ticket->ticket_number + offset));
	ticket->seq_number = seq_number++;
}

unsigned int random_set_of_tickets(char *ticket_set, unsigned  long long  number)
{
	unsigned long long  idx = 0;
	ticket *temp_ticket = NULL;

	assert(ticket_set);
	temp_ticket	= (ticket *)ticket_set;
	
	for(idx = 0; idx < number; idx++)
	{
		(void)random_a_ticket(temp_ticket);
		temp_ticket++;
	}
}

void print_tickets(char *src, unsigned long long numbers)
{
	unsigned long long idx = 0, idx_in = 0;
	ticket *temp_ticket = NULL;

	temp_ticket = (ticket *)src;
	for(idx = 0; idx < numbers; idx++)
	{
		printf("+-----------------------------------\n");
		printf("|SN:%#llx\n", temp_ticket->seq_number);
		printf("|R number:");
		for(idx_in = 0; idx_in < MAX_SIZE; idx_in++)
		{
			printf(" %02d ", temp_ticket->ticket_number[idx_in]);
		}
		printf("\n|B number: %02d \n", temp_ticket->ticket_number[MAX_SIZE]);
		printf("+-----------------------------------\n\n");

		temp_ticket++;
	}
}

unsigned long long choice_one_number(unsigned long long total_tickets)
{
	unsigned long long base = total_tickets;
	unsigned long long result = 0;

	result = 1 + (rand() % base);
	
	return result;	
}

void choice_one_ticket(unsigned long long number, unsigned long long choice_seq)
{
	ticket *temp_ticket = global_ticket;
	while(temp_ticket->seq_number != number)
		temp_ticket->seq_number++;
	choiced_seq_addr[choice_seq] = (unsigned long long)temp_ticket;
}


char find_uint_in_array(unsigned long long finder, unsigned long long *array, unsigned long long size)
{
	unsigned long long loop_idx = 0;
	
	assert(array);

	for(loop_idx = 0; loop_idx < size; loop_idx++)
	{
		if(finder == *array++)
			return S_OK;
	}

	return S_FAILED;
}

void choice_tickets(unsigned long long product_number, unsigned long long choice_number)
{
	unsigned long long result = 0, idx = 0;
	
	for(idx = 0; idx < choice_number; idx++)
	{
		result = choice_one_number(product_number);
		if(S_OK == find_uint_in_array(result, choice_seq, idx))
		{
			idx--;
			continue;
		}
		choice_seq[idx] = result;
	}

	for(idx = 0; idx < choice_number; idx++)
	{
		choice_one_ticket(choice_seq[idx], idx);
	}
}

void print_choiced_tickets(unsigned long long choice_number)
{
	unsigned long long idx = 0;
	char idx_in = 0;
	ticket *temp_ticket;
	
	for(idx = 0; idx < choice_number; idx++)
	{
		temp_ticket =  (ticket *)(choiced_seq_addr[idx]);
		printf("+-----------------------------------\n");
		printf("|SN:%lld\n", temp_ticket->seq_number);
		printf("|R number:");
		for(idx_in = 0; idx_in < MAX_SIZE; idx_in++)
		{
			printf(" %02d ", temp_ticket->ticket_number[idx_in]);
		}
		printf("\n|B number: %02d \n", temp_ticket->ticket_number[MAX_SIZE]);
		printf("+-----------------------------------\n\n");
	}
}


static unsigned long long counter[34] = {0, };
void number_counter(ticket *base_addr, unsigned long long tickets)
{
	unsigned long long idx = 0;
	unsigned char idy = 0, idz = 0;
	ticket *temp = base_addr;

	for(idx = 0; idx < tickets; idx++)
	{
		for(idy = 0; idy < 6; idy++)
		{
			for(idz = 1; idz <= 33; idz++)
			{
				if(idz == temp->ticket_number[idy])
				{
					counter[idz]++;
				}
			}
		}
		
		temp++;
	}

	for(idz = 1; idz <= 33; idz++)
	{
		printf("Number[%02d] = 0x%llx\n", idz, counter[idz]);
	}
}

int main(int argc, char *argv[])
{
	long long product_number = 0;
	long long choice_number = 10;
	char idx = 0;
	
	srand((int)time(0));

	if(argc != 3)
	{
		printf("Usage:\n\t%s [All tickets] [Choice ticket]\n", argv[0]);
		return 0;
	}
	
	product_number = atoi(argv[1]);
	if(product_number <= 0)
		product_number = 10000000;
		
	choice_number = atoi(argv[2]);
	if(choice_number <= 0)
		choice_number = 10;

	if(choice_number > product_number)
	{
		printf("Choice ticket is overflow\n");
		return 0;
	}
	while(NULL == (global_ticket = (ticket *)malloc(product_number * (sizeof(ticket)))));
	choiced_seq_addr = (unsigned long long *)malloc(choice_number * sizeof(unsigned long long));
	
	random_set_of_tickets((char *)global_ticket, product_number);

	choice_seq = (unsigned long long *)malloc(choice_number * sizeof(unsigned long long));
	memset(choice_seq, 0, (choice_number * sizeof(unsigned long long)));

	choice_tickets(product_number, choice_number);
	print_choiced_tickets(choice_number);

	//number_counter(global_ticket,(seq_number - 1));
	
	free(global_ticket);
	free(choiced_seq_addr);
	free(choice_seq);

	return 0;
}



