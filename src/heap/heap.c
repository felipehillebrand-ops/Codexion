/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 10:08:27 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/02 06:58:12 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_push(t_heap *heap, long key, int coder_id)
{
	if (heap->size >= heap->capacity)
		return (fprintf(stderr, "Error: heap is full\n"), -1);
	heap->nodes[heap->size].key = key;
	heap->nodes[heap->size].coder_id = coder_id;
	heap->size++;
	heapify_up(heap, heap->size - 1);
	return (0);
}

int	heap_pop(t_heap *heap, t_heap_node *out)
{
	if (heap->size == 0)
		return (-1);
	*out = heap->nodes[0];
	heap->size--;
	heap->nodes[0] = heap->nodes[heap->size];
	if (heap->size > 0)
		heapify_down(heap, 0);
	return (0);
}
