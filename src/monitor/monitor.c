/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 15:22:17 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/07 15:21:45 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_burnout(t_data *data, int i)
{
	t_coder	*coder;
	long	now;
	int		burned;

	coder = &data->coders[i];
	pthread_mutex_lock(&data->log_lock);
	pthread_mutex_lock(&coder->progress_lock);
	now = get_timestamp_ms(data);
	burned = !is_simulation_stopped(data)
		&& coder->compiles_done < data->number_of_compiles_required
		&& now - coder->last_compile_start >= data->time_to_burnout;
	if (burned)
	{
		set_simulation_stopped(data);
		printf("%ld %d burned out\n", now, coder->id);
	}
	pthread_mutex_unlock(&coder->progress_lock);
	pthread_mutex_unlock(&data->log_lock);
	return (burned);
}

static int	check_all_done(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		if (coder_get_compiles_done(&data->coders[i])
			< data->number_of_compiles_required)
			return (0);
		i++;
	}
	return (1);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		i;

	data = (t_data *)arg;
	if (!wait_for_start(data))
		return (NULL);
	while (!is_simulation_stopped(data))
	{
		i = 0;
		while (i < data->number_of_coders && !is_simulation_stopped(data))
		{
			if (check_burnout(data, i))
				break ;
			i++;
		}
		if (check_all_done(data))
			set_simulation_stopped(data);
		usleep(MONITOR_POLL_MS * 1000);
	}
	wake_all_dongles(data);
	return (NULL);
}
