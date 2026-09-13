/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_queue.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/12 17:04:06 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/13 19:56:31 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	compute_request_key(t_coder *coder, t_dongle *dongle)
{
	if (coder->data->scheduler == CX_SCHED_FIFO)
		return (dongle->request_counter++);
	return (coder_get_last_compile_start(coder) + coder->data->time_to_burnout);
}

int	dongle_enqueue(t_coder *coder, t_dongle *dongle)
{
	long	key;
	int		ret;

	ret = 0;
	pthread_mutex_lock(&dongle->lock);
	if (!is_simulation_stopped(coder->data))
	{
		key = compute_request_key(coder, dongle);
		ret = heap_push(&dongle->waiting_queue, key, coder->id);
		if (ret == 0)
			pthread_cond_broadcast(&dongle->cond);
	}
	pthread_mutex_unlock(&dongle->lock);
	return (ret);
}

void	dongle_dequeue(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->lock);
	heap_remove_by_id(&dongle->waiting_queue, coder->id);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}
