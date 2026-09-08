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

#define DONGLE_POLL_MS 50

static long	compute_request_key(t_coder *coder, t_dongle *dongle)
{
	if (coder->data->scheduler == CX_SCHED_FIFO)
		return (dongle->request_counter++);
	return (coder_get_last_compile_start(coder) + coder->data->time_to_burnout);
}

static int	dongle_is_ready(t_data *data, t_dongle *dongle, int coder_id)
{
	if (dongle->waiting_queue.size == 0)
		return (0);
	if (dongle->waiting_queue.nodes[0].coder_id != coder_id)
		return (0);
	if (!dongle->is_available)
		return (0);
	if (get_timestamp_ms(data) < dongle->available_at_ms)
		return (0);
	return (1);
}

static int	dongle_wait_loop(t_data *data, t_dongle *dongle, int coder_id)
{
	struct timespec	ts;
	long			wake_ms;

	while (!dongle_is_ready(data, dongle, coder_id))
	{
		if (is_simulation_stopped(data))
			return (-1);
		if (dongle->is_available
			&& get_timestamp_ms(data) < dongle->available_at_ms)
			wake_ms = dongle->available_at_ms;
		else
			wake_ms = get_timestamp_ms(data) + DONGLE_POLL_MS;
		ms_to_abstime(data, wake_ms, &ts);
		pthread_cond_timedwait(&dongle->cond, &dongle->lock, &ts);
	}
	return (0);
}

int	dongle_acquire_single(t_coder *coder, t_dongle *dongle)
{
	long		key;
	t_heap_node	popped;

	pthread_mutex_lock(&dongle->lock);
	key = compute_request_key(coder, dongle);
	if (heap_push(&dongle->waiting_queue, key, coder->id) != 0)
		return (pthread_mutex_unlock(&dongle->lock), -1);
	if (dongle_wait_loop(coder->data, dongle, coder->id) != 0)
		return (pthread_mutex_unlock(&dongle->lock), -1);
	heap_pop(&dongle->waiting_queue, &popped);
	dongle->is_available = 0;
	pthread_mutex_unlock(&dongle->lock);
	log_dongle_taken(coder->data, coder->id);
	return (0);
}

int	coder_acquire_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->left_dongle == coder->right_dongle)
		return (dongle_acquire_single(coder, coder->left_dongle));
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		first = coder->left_dongle;
		second = coder->right_dongle;
	}
	else
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	if (dongle_acquire_single(coder, first) != 0)
		return (-1);
	if (dongle_acquire_single(coder, second) != 0)
	{
		dongle_release_single(coder, first);
		return (-1);
	}
	return (0);
}
