/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_pair.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 00:23:26 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/16 00:48:07 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	lock_pair(t_coder *coder)
{
	if (coder->left_dongle->id < coder->right_dongle->id)
	{
		pthread_mutex_lock(&coder->left_dongle->lock);
		pthread_mutex_lock(&coder->right_dongle->lock);
	}
	else
	{
		pthread_mutex_lock(&coder->right_dongle->lock);
		if (coder->left_dongle != coder->right_dongle)
			pthread_mutex_lock(&coder->left_dongle->lock);
	}
}

static int	dongle_ready(t_dongle *dongle, int id, long now)
{
	if (dongle->waiting_queue.size == 0)
		return (0);
	return (dongle->is_available && now >= dongle->available_at_ms
		&& dongle->waiting_queue.nodes[0].coder_id == id);
}

static void	reserve_pair(t_coder *coder)
{
	t_heap_node	node;

	heap_pop(&coder->left_dongle->waiting_queue, &node);
	coder->left_dongle->is_available = 0;
	if (coder->left_dongle != coder->right_dongle)
	{
		heap_pop(&coder->right_dongle->waiting_queue, &node);
		coder->right_dongle->is_available = 0;
	}
}

static void	set_wake_time(t_coder *coder, long now, long *wake)
{
	*wake = LONG_MAX;
	if (now <= LONG_MAX - DONGLE_POLL_MS)
		*wake = now + DONGLE_POLL_MS;
	if (coder->left_dongle->available_at_ms > now
		&& coder->left_dongle->available_at_ms < *wake)
		*wake = coder->left_dongle->available_at_ms;
	if (coder->right_dongle->available_at_ms > now
		&& coder->right_dongle->available_at_ms < *wake)
		*wake = coder->right_dongle->available_at_ms;
}

int	coder_try_pair(t_coder *coder, long *wake_ms)
{
	long	now;
	int		ready;

	lock_pair(coder);
	now = get_timestamp_ms(coder->data);
	set_wake_time(coder, now, wake_ms);
	ready = dongle_ready(coder->left_dongle, coder->id, now)
		&& dongle_ready(coder->right_dongle, coder->id, now);
	if (ready)
		reserve_pair(coder);
	pthread_mutex_unlock(&coder->left_dongle->lock);
	if (coder->left_dongle != coder->right_dongle)
		pthread_mutex_unlock(&coder->right_dongle->lock);
	return (ready);
}
