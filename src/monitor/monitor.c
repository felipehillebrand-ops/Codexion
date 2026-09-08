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

#define MONITOR_POLL_MS 1

static int	check_burnout(t_data *data, int i)
{
	long	elapsed;
	long	since;

	since = coder_get_last_compile_start(&data->coders[i]);
	elapsed = get_timestamp_ms(data) - since;
	if (elapsed >= data->time_to_burnout)
	{
		log_state_change(data, data->coders[i].id, STATE_BURNED_OUT);
		set_simulation_stopped(data);
		return (1);
	}
	return (0);
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
