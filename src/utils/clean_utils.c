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
	while (i < data->number_of_coders)
	{
		pthread_mutex_destroy(&data->dongles[i].lock);
		pthread_cond_destroy(&data->dongles[i].cond);
		free(data->dongles[i].waiting_queue.nodes);
		i++;
	}
	free(data->dongles);
}

static void	destroy_coders(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
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
	pthread_mutex_destroy(&data->log_lock);
	pthread_mutex_destroy(&data->stop_lock);
}
