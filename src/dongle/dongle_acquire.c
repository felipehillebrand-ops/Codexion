/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_acquire.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/01 10:41:08 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/01 11:48:14 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	wait_for_pair(t_coder *coder)
{
	struct timespec	ts;
	long			wake_ms;
	int				taken;

	taken = 0;
	pthread_mutex_lock(&coder->data->request_lock);
	while (!is_simulation_stopped(coder->data) && !taken)
	{
		taken = coder_try_pair(coder, &wake_ms);
		if (!taken)
		{
			ms_to_abstime(coder->data, wake_ms, &ts);
			pthread_cond_timedwait(&coder->data->request_cond,
				&coder->data->request_lock, &ts);
		}
	}
	if (!taken)
		coder_drop_request(coder);
	pthread_cond_broadcast(&coder->data->request_cond);
	pthread_mutex_unlock(&coder->data->request_lock);
	return (!taken);
}

static int	wait_single(t_coder *coder)
{
	log_dongle_taken(coder->data, coder->id);
	while (!is_simulation_stopped(coder->data))
		usleep(500);
	dongle_release_single(coder, coder->left_dongle);
	return (-1);
}

int	coder_acquire_dongles(t_coder *coder)
{
	if (coder_get_compiles_done(coder) == 0)
	{
		if (!wait_initial_slot(coder))
			return (-1);
	}
	else if (coder_queue_request(coder) != 0)
		return (-1);
	if (wait_for_pair(coder) != 0)
		return (-1);
	if (coder->left_dongle == coder->right_dongle)
		return (wait_single(coder));
	return (0);
}
