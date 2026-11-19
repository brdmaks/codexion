/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_order.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	cx_edf_tie_break(t_request a, t_request b)
{
	if (a.coder_id != b.coder_id)
		return (a.coder_id < b.coder_id);
	return (a.sequence < b.sequence);
}

int	cx_request_has_priority(t_request a, t_request b, t_sched sched)
{
	if (sched == CX_SCHED_FIFO)
	{
		if (a.sequence != b.sequence)
			return (a.sequence < b.sequence);
		return (a.coder_id < b.coder_id);
	}
	if (a.deadline != b.deadline)
		return (a.deadline < b.deadline);
	return (cx_edf_tie_break(a, b));
}

void	cx_heap_sift_up(t_heap *heap, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (cx_request_has_priority(heap->arr[idx], heap->arr[parent],
				heap->sched))
		{
			cx_heap_swap(&heap->arr[idx], &heap->arr[parent]);
			idx = parent;
		}
		else
			break ;
	}
}

void	cx_heap_sift_down(t_heap *heap, int idx)
{
	int	left;
	int	right;
	int	best;

	while ((2 * idx + 1) < heap->size)
	{
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		best = idx;
		if (cx_request_has_priority(heap->arr[left], heap->arr[best],
				heap->sched))
			best = left;
		if (right < heap->size
			&& cx_request_has_priority(heap->arr[right], heap->arr[best],
				heap->sched))
			best = right;
		if (best == idx)
			break ;
		cx_heap_swap(&heap->arr[idx], &heap->arr[best]);
		idx = best;
	}
}
