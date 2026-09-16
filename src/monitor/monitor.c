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

static t_coder	*earliest_coder(t_data *data)
{
	t_coder	*first;
	int		i;

	first = &data->coders[0];
	i = 1;
	while (i < data->number_of_coders)
	{
		if (coder_get_last_compile_start(&data->coders[i])
			< coder_get_last_compile_start(first))
			first = &data->coders[i];
		i++;
	}
	return (first);
}

static void	check_state(t_data *data)
{
	t_coder	*coder;
	long	check_at;

	if (is_simulation_stopped(data))
		return ;
	coder = earliest_coder(data);
	check_at = get_timestamp_ms(data);
	if (data->coders_finished == data->number_of_coders)
		check_at = data->finished_at_ms;
	if (data->number_of_compiles_required > 0
		&& check_at - coder_get_last_compile_start(coder)
		>= data->time_to_burnout)
	{
		set_simulation_stopped(data);
		printf("%ld %d burned out\n", get_timestamp_ms(data), coder->id);
	}
	else if (data->number_of_compiles_required == 0
		|| data->coders_finished == data->number_of_coders)
		set_simulation_stopped(data);
}

void	*monitor_routine(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	if (!wait_for_start(data))
		return (NULL);
	while (!is_simulation_stopped(data))
	{
		pthread_mutex_lock(&data->log_lock);
		check_state(data);
		pthread_mutex_unlock(&data->log_lock);
		usleep(MONITOR_POLL_MS * 1000);
	}
	wake_all_dongles(data);
	return (NULL);
}
