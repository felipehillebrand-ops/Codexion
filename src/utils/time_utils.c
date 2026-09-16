/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/19 09:55:48 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/01 10:39:41 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	record_start_time(t_data *data)
{
	gettimeofday(&data->start_time, NULL);
}

long	get_timestamp_ms(t_data *data)
{
	struct timeval	now;
	long			sec_diff;
	long			usec_diff;

	gettimeofday(&now, NULL);
	sec_diff = now.tv_sec - data->start_time.tv_sec;
	usec_diff = now.tv_usec - data->start_time.tv_usec;
	return ((sec_diff * 1000000 + usec_diff) / 1000);
}

void	ms_to_abstime(t_data *data, long target_ms, struct timespec *ts)
{
	long	usec;

	usec = data->start_time.tv_usec + (target_ms % 1000) * 1000;
	ts->tv_sec = data->start_time.tv_sec + target_ms / 1000 + usec / 1000000;
	ts->tv_nsec = (usec % 1000000) * 1000;
}

int	sleep_ms(t_data *data, long duration)
{
	struct timeval	start;
	struct timeval	now;
	long			elapsed;

	gettimeofday(&start, NULL);
	while (!is_simulation_stopped(data))
	{
		gettimeofday(&now, NULL);
		elapsed = (now.tv_sec - start.tv_sec) * 1000000;
		elapsed += now.tv_usec - start.tv_usec;
		if (elapsed / 1000 >= duration)
			return (1);
		usleep(500);
	}
	return (0);
}
