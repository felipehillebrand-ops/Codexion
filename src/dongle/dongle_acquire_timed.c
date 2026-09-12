/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_acquire_timed.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 17:04:06 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/12 21:21:51 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	order_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		*first = coder->left_dongle;
		*second = coder->right_dongle;
	}
	else
	{
		*first = coder->right_dongle;
		*second = coder->left_dongle;
	}
}

static int	dongle_wait_loop_bounded(t_data *data, t_dongle *dongle,
		int coder_id, long deadline_ms)
{
	struct timespec	ts;
	long			now;
	long			wake_ms;

	while (!dongle_is_ready(data, dongle, coder_id))
	{
		if (is_simulation_stopped(data))
			return (-1);
		now = get_timestamp_ms(data);
		if (now >= deadline_ms)
			return (1);
		wake_ms = now + DONGLE_POLL_MS;
		if (dongle->is_available && dongle->available_at_ms < wake_ms)
			wake_ms = dongle->available_at_ms;
		if (wake_ms > deadline_ms)
			wake_ms = deadline_ms;
		ms_to_abstime(data, wake_ms, &ts);
		pthread_cond_timedwait(&dongle->cond, &dongle->lock, &ts);
	}
	return (0);
}

int	dongle_acquire_single_timed(t_coder *coder, t_dongle *dongle,
		long timeout_ms)
{
	long		key;
	long		deadline;
	t_heap_node	popped;
	int			wait_result;

	pthread_mutex_lock(&dongle->lock);
	if (is_simulation_stopped(coder->data))
		return (pthread_mutex_unlock(&dongle->lock), -1);
	key = compute_request_key(coder, dongle);
	if (heap_push(&dongle->waiting_queue, key, coder->id) != 0)
		return (pthread_mutex_unlock(&dongle->lock), -1);
	deadline = get_timestamp_ms(coder->data) + timeout_ms;
	wait_result = dongle_wait_loop_bounded(coder->data, dongle,
			coder->id, deadline);
	if (wait_result != 0)
	{
		heap_remove_by_id(&dongle->waiting_queue, coder->id);
		pthread_mutex_unlock(&dongle->lock);
		return (wait_result);
	}
	heap_pop(&dongle->waiting_queue, &popped);
	dongle->is_available = 0;
	pthread_mutex_unlock(&dongle->lock);
	log_dongle_taken(coder->data, coder->id);
	return (0);
}
