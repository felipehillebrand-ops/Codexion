/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clean_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 15:12:49 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/07 15:11:32 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_dongles(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->dongles_ready)
	{
		pthread_mutex_destroy(&data->dongles[i].lock);
		free(data->dongles[i].waiting_queue.nodes);
		i++;
	}
	free(data->dongles);
}

static void	destroy_coders(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->coders_ready)
	{
		pthread_mutex_destroy(&data->coders[i].progress_lock);
		i++;
	}
	free(data->coders);
}

void	clean_data(t_data *data)
{
	destroy_coders(data);
	destroy_dongles(data);
	if (data->sync_ready >= 5)
		pthread_cond_destroy(&data->request_cond);
	if (data->sync_ready >= 4)
		pthread_mutex_destroy(&data->request_lock);
	if (data->sync_ready >= 3)
		pthread_cond_destroy(&data->start_cond);
	if (data->sync_ready >= 2)
		pthread_mutex_destroy(&data->stop_lock);
	if (data->sync_ready >= 1)
		pthread_mutex_destroy(&data->log_lock);
}
