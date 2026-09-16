/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 10:09:05 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/02 06:58:16 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	heap_swap(t_heap_node *a, t_heap_node *b)
{
	t_heap_node	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	heap_has_priority(t_heap_node *a, t_heap_node *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	if (a->order != b->order)
		return (a->order < b->order);
	return (a->coder_id < b->coder_id);
}

void	heapify_up(t_heap *heap, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (!heap_has_priority(&heap->nodes[idx], &heap->nodes[parent]))
			break ;
		heap_swap(&heap->nodes[idx], &heap->nodes[parent]);
		idx = parent;
	}
}

void	heapify_down(t_heap *heap, int idx)
{
	int	left;
	int	right;
	int	best;

	left = idx * 2 + 1;
	right = idx * 2 + 2;
	best = idx;
	if (left < heap->size && heap_has_priority(&heap->nodes[left],
			&heap->nodes[best]))
		best = left;
	if (right < heap->size && heap_has_priority(&heap->nodes[right],
			&heap->nodes[best]))
		best = right;
	if (best != idx)
	{
		heap_swap(&heap->nodes[idx], &heap->nodes[best]);
		heapify_down(heap, best);
	}
}
