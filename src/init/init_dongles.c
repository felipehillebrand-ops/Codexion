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

#define DONGLE_QUEUE_CAP 2

static int	init_single_dongle(t_dongle *dongle, int id)
{
	dongle->id = id;
	if (pthread_mutex_init(&dongle->lock, NULL) != 0)
		return (fprintf(stderr, "Error: mutex init failed\n"), -1);
	if (pthread_cond_init(&dongle->cond, NULL) != 0)
		return (fprintf(stderr, "Error: cond init failed\n"), -1);
	dongle->is_available = 1;
	dongle->available_at_ms = 0;
	dongle->request_counter = 0;
	dongle->waiting_queue.nodes = malloc(
			sizeof(t_heap_node) * DONGLE_QUEUE_CAP);
	if (!dongle->waiting_queue.nodes)
		return (fprintf(stderr, "Error: malloc failed for heap\n"), -1);
	dongle->waiting_queue.size = 0;
	dongle->waiting_queue.capacity = DONGLE_QUEUE_CAP;
	return (0);
}

static void	destroy_dongles_up_to(t_data *data, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&data->dongles[i].lock);
		pthread_cond_destroy(&data->dongles[i].cond);
		free(data->dongles[i].waiting_queue.nodes);
		i++;
	}
	free(data->dongles);
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
	int	i;

	data->dongles = malloc(sizeof(t_dongle) * data->number_of_coders);
	if (!data->dongles)
		return (fprintf(stderr, "Error: malloc failed for dongles\n"), -1);
	i = 0;
	while (i < data->number_of_coders)
	{
		if (init_single_dongle(&data->dongles[i], i) != 0)
			return (destroy_dongles_up_to(data, i), -1);
		i++;
	}
	link_coder_dongles(data);
	return (0);
}
