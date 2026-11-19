/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	cx_heap_init(t_heap *heap, int capacity, t_sched sched)
{
	heap->capacity = capacity;
	heap->size = 0;
	heap->sched = sched;
	heap->arr = (t_request *)malloc(sizeof(t_request) * capacity);
	if (!heap->arr)
		return (0);
	return (1);
}

void	cx_heap_destroy(t_heap *heap)
{
	if (heap->arr)
	{
		free(heap->arr);
		heap->arr = NULL;
	}
	heap->size = 0;
	heap->capacity = 0;
}

void	cx_heap_swap(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	cx_heap_push(t_heap *heap, t_request req)
{
	if (heap->size >= heap->capacity)
		return (0);
	heap->arr[heap->size] = req;
	cx_heap_sift_up(heap, heap->size);
	heap->size++;
	return (1);
}

int	cx_heap_pop(t_heap *heap, t_request *out_req)
{
	if (heap->size <= 0)
		return (0);
	*out_req = heap->arr[0];
	heap->size--;
	if (heap->size > 0)
	{
		heap->arr[0] = heap->arr[heap->size];
		cx_heap_sift_down(heap, 0);
	}
	return (1);
}
