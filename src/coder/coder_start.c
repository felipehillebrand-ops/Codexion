/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_start.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:54:33 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/16 01:02:10 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	initial_id(t_data *data)
{
	int	odds;

	odds = data->number_of_coders / 2 + data->number_of_coders % 2;
	if (data->initial_queued < odds)
		return (data->initial_queued * 2 + 1);
	return ((data->initial_queued - odds) * 2 + 2);
}

static int	register_initial(t_coder *coder)
{
	if (coder_queue_request(coder) != 0)
	{
		set_simulation_stopped(coder->data);
		return (0);
	}
	pthread_mutex_lock(&coder->data->stop_lock);
	coder->data->initial_queued++;
	pthread_cond_broadcast(&coder->data->start_cond);
	pthread_mutex_unlock(&coder->data->stop_lock);
	return (wait_for_start(coder->data));
}

int	prepare_coder(t_coder *coder)
{
	t_data	*data;

	data = coder->data;
	pthread_mutex_lock(&data->stop_lock);
	while (!data->simulation_stopped && initial_id(data) != coder->id)
		pthread_cond_wait(&data->start_cond, &data->stop_lock);
	pthread_mutex_unlock(&data->stop_lock);
	if (is_simulation_stopped(data))
		return (0);
	return (register_initial(coder));
}

static long	initial_delay(t_coder *coder)
{
	long		span;
	long		rank;
	long long	rest;
	int			half;

	half = coder->data->number_of_coders / 2;
	if (half == 0 || coder->id == 1)
		return (0);
	span = coder->data->time_to_compile;
	if (span > LONG_MAX - coder->data->dongle_cooldown)
		return (LONG_MAX);
	span += coder->data->dongle_cooldown;
	if (coder->data->number_of_coders % 2 == 0)
		return (span * (coder->id % 2 == 0));
	rank = (coder->id - 1) / 2;
	if (coder->id % 2 == 0)
		rank += half + 1;
	if (span > LONG_MAX / 2)
		return (LONG_MAX);
	rest = span % half;
	return ((span / half) * rank + rest * rank / half);
}

int	wait_initial_slot(t_coder *coder)
{
	long	delay;
	long	now;

	delay = initial_delay(coder);
	now = get_timestamp_ms(coder->data);
	if (delay > now)
		return (sleep_ms(coder->data, delay - now));
	return (!is_simulation_stopped(coder->data));
}
