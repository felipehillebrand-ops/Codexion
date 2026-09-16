/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_queue.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 17:04:06 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/16 01:17:13 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	request_key(t_coder *coder, long order)
{
	long	last;

	if (coder->data->scheduler == CX_SCHED_FIFO)
		return (order);
	last = coder_get_last_compile_start(coder);
	if (last > LONG_MAX - coder->data->time_to_burnout)
		return (LONG_MAX);
	return (last + coder->data->time_to_burnout);
}

static int	push_request(t_dongle *dongle, long key, long order, int id)
{
	int	ret;

	pthread_mutex_lock(&dongle->lock);
	ret = heap_push(&dongle->waiting_queue, key, order, id);
	pthread_mutex_unlock(&dongle->lock);
	return (ret);
}

void	coder_drop_request(t_coder *coder)
{
	pthread_mutex_lock(&coder->left_dongle->lock);
	heap_remove_by_id(&coder->left_dongle->waiting_queue, coder->id);
	pthread_mutex_unlock(&coder->left_dongle->lock);
	if (coder->left_dongle == coder->right_dongle)
		return ;
	pthread_mutex_lock(&coder->right_dongle->lock);
	heap_remove_by_id(&coder->right_dongle->waiting_queue, coder->id);
	pthread_mutex_unlock(&coder->right_dongle->lock);
}

static int	push_pair(t_coder *coder, long order)
{
	long	key;
	int		ret;

	key = request_key(coder, order);
	ret = push_request(coder->left_dongle, key, order, coder->id);
	if (ret == 0 && coder->left_dongle != coder->right_dongle)
		ret = push_request(coder->right_dongle, key, order, coder->id);
	if (ret != 0)
		coder_drop_request(coder);
	return (ret);
}

int	coder_queue_request(t_coder *coder)
{
	int	ret;

	ret = -1;
	pthread_mutex_lock(&coder->data->request_lock);
	if (!is_simulation_stopped(coder->data))
		ret = push_pair(coder, coder->data->request_counter++);
	pthread_cond_broadcast(&coder->data->request_cond);
	pthread_mutex_unlock(&coder->data->request_lock);
	return (ret);
}
