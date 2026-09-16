/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_dongles.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/08 18:55:21 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/08 19:00:37 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_single_dongle(t_dongle *dongle, int id)
{
	dongle->waiting_queue.nodes = malloc(
			sizeof(t_heap_node) * DONGLE_QUEUE_CAP);
	if (!dongle->waiting_queue.nodes)
		return (-1);
	if (pthread_mutex_init(&dongle->lock, NULL) != 0)
		return (free(dongle->waiting_queue.nodes), -1);
	dongle->id = id;
	dongle->is_available = 1;
	dongle->available_at_ms = 0;
	dongle->waiting_queue.size = 0;
	dongle->waiting_queue.capacity = DONGLE_QUEUE_CAP;
	return (0);
}

static void	link_coder_dongles(t_data *data)
{
	int	i;
	int	next;

	i = 0;
	while (i < data->number_of_coders)
	{
		next = (i + 1) % data->number_of_coders;
		data->coders[i].right_dongle = &data->dongles[i];
		data->coders[next].left_dongle = &data->dongles[i];
		i++;
	}
}

int	init_dongles(t_data *data)
{
	data->dongles = malloc(sizeof(t_dongle) * data->number_of_coders);
	if (!data->dongles)
		return (-1);
	while (data->dongles_ready < data->number_of_coders)
	{
		if (init_single_dongle(&data->dongles[data->dongles_ready],
				data->dongles_ready) != 0)
			return (-1);
		data->dongles_ready++;
	}
	link_coder_dongles(data);
	return (0);
}
