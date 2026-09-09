/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_data_coders.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 10:12:13 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/02 04:21:52 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
