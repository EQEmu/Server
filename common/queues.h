/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free
	Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
	MA 02111-1307, USA */

/*
	Code for generell handling of priority Queues.
	Implemention of queues from "Algoritms in C" by Robert Sedgewick.
	Copyright Monty Program KB.
	By monty.
*/

#ifndef _queues_h
#define _queues_h
#ifdef	__cplusplus
extern "C" {
#endif

typedef struct st_queue {
	byte **root;
	void *first_cmp_arg;
	uint elements;
	uint max_elements;
	uint offset_to_key;			/* compare is done on element+offset */
	int max_at_top;			/* Set if queue_top gives max */
	int (*compare)(void *, byte *,byte *);
} QUEUE;

#define queue_top(queue) ((queue)->root[1])
#define queue_element(queue,index) ((queue)->root[index+1])
#define queue_end(queue) ((queue)->root[(queue)->elements])
#define queue_replaced(queue) _downheap(queue,1)

int init_queue(QUEUE *queue,uint max_elements,uint offset_to_key,
			pbool max_at_top, int (*compare)(void *,byte *, byte *),
			void *first_cmp_arg);
int reinit_queue(QUEUE *queue,uint max_elements,uint offset_to_key,
			pbool max_at_top, int (*compare)(void *,byte *, byte *),
			void *first_cmp_arg);
void delete_queue(QUEUE *queue);
void queue_insert(QUEUE *queue,byte *element);
byte *queue_remove(QUEUE *queue,uint idx);
void _downheap(QUEUE *queue,uint idx);
#define is_queue_inited(queue) ((queue)->root != 0)

#ifdef	__cplusplus
}
#endif
#endif
