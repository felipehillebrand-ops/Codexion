/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_state.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/01 10:32:17 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/07 11:35:29 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_simulation_stopped(t_data *data)
{
	int	stopped;

	pthread_mutex_lock(&data->stop_lock);
	stopped = data->simulation_stopped;
	pthread_mutex_unlock(&data->stop_lock);
	return (stopped);
}

void	set_simulation_stopped(t_data *data)
{
	pthread_mutex_lock(&data->stop_lock);
	data->simulation_stopped = 1;
	pthread_cond_broadcast(&data->start_cond);
	pthread_mutex_unlock(&data->stop_lock);
}

int	wait_for_start(t_data *data)
{
	int	started;

	pthread_mutex_lock(&data->stop_lock);
	while (!data->simulation_started && !data->simulation_stopped)
		pthread_cond_wait(&data->start_cond, &data->stop_lock);
	started = !data->simulation_stopped;
	pthread_mutex_unlock(&data->stop_lock);
	return (started);
}

void	start_simulation(t_data *data)
{
	pthread_mutex_lock(&data->stop_lock);
	while (data->initial_queued < data->number_of_coders
		&& !data->simulation_stopped)
		pthread_cond_wait(&data->start_cond, &data->stop_lock);
	if (!data->simulation_stopped)
	{
		record_start_time(data);
		data->simulation_started = 1;
	}
	pthread_cond_broadcast(&data->start_cond);
	pthread_mutex_unlock(&data->stop_lock);
}
