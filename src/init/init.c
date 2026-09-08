/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 10:12:13 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/02 04:21:52 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

#define DONGLE_QUEUE_CAP 2

static int	init_coders(t_data *data)
{
	int	i;

	data->coders = malloc(sizeof(t_coder) * data->number_of_coders);
	if (!data->coders)
		return (fprintf(stderr, "Error: malloc failed for coders\n"), -1);
	i = 0;
	while (i < data->number_of_coders)
	{
		data->coders[i].id = i + 1;
		data->coders[i].state = STATE_IDLE;
		data->coders[i].compiles_done = 0;
		data->coders[i].last_compile_start = 0;
		pthread_mutex_init(&data->coders[i].progress_lock, NULL);
		data->coders[i].left_dongle = NULL;
		data->coders[i].right_dongle = NULL;
		data->coders[i].data = data;
		i++;
	}
	return (0);
}

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
	dongle->waiting_queue.nodes = malloc(sizeof(t_heap_node) * DONGLE_QUEUE_CAP);
	if (!dongle->waiting_queue.nodes)
		return (fprintf(stderr, "Error: malloc failed for heap\n"), -1);
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

static int	init_dongles(t_data *data)
{
	int	i;

	data->dongles = malloc(sizeof(t_dongle) * data->number_of_coders);
	if (!data->dongles)
		return (fprintf(stderr, "Error: malloc failed for dongles\n"), -1);
	i = 0;
	while (i < data->number_of_coders)
	{
		if (init_single_dongle(&data->dongles[i], i) != 0)
			return (-1);
		i++;
	}
	link_coder_dongles(data);
	return (0);
}

int	init_data(t_data *data)
{
	record_start_time(data);
	if (pthread_mutex_init(&data->log_lock, NULL) != 0)
		return (fprintf(stderr, "Error: log_lock init failed\n"), -1);
	if (pthread_mutex_init(&data->stop_lock, NULL) != 0)
		return (fprintf(stderr, "Error: stop_lock init failed\n"), -1);
	data->simulation_stopped = 0;
	if (init_coders(data) != 0)
		return (-1);
	if (init_dongles(data) != 0)
		return (-1);
	return (0);
}
