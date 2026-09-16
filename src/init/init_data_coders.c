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

static int	init_sync(t_data *data)
{
	if (pthread_mutex_init(&data->log_lock, NULL) != 0)
		return (-1);
	data->sync_ready = 1;
	if (pthread_mutex_init(&data->stop_lock, NULL) != 0)
		return (-1);
	data->sync_ready = 2;
	if (pthread_cond_init(&data->start_cond, NULL) != 0)
		return (-1);
	data->sync_ready = 3;
	if (pthread_mutex_init(&data->request_lock, NULL) != 0)
		return (-1);
	data->sync_ready = 4;
	if (pthread_cond_init(&data->request_cond, NULL) != 0)
		return (-1);
	data->sync_ready = 5;
	return (0);
}

static int	init_coders(t_data *data)
{
	t_coder	*coder;

	data->coders = malloc(sizeof(t_coder) * data->number_of_coders);
	if (!data->coders)
		return (-1);
	memset(data->coders, 0, sizeof(t_coder) * data->number_of_coders);
	while (data->coders_ready < data->number_of_coders)
	{
		coder = &data->coders[data->coders_ready];
		coder->id = data->coders_ready + 1;
		coder->data = data;
		if (pthread_mutex_init(&coder->progress_lock, NULL) != 0)
			return (-1);
		data->coders_ready++;
	}
	return (0);
}

int	init_data(t_data *data)
{
	if (init_sync(data) != 0 || init_coders(data) != 0
		|| init_dongles(data) != 0)
		return (-1);
	return (0);
}
