/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_release.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/01 10:41:15 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/07 11:48:19 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	dongle_release_single(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->lock);
	dongle->is_available = 1;
	dongle->available_at_ms = get_timestamp_ms(coder->data)
		+ coder->data->dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}

void	coder_release_dongles(t_coder *coder)
{
	dongle_release_single(coder, coder->left_dongle);
	dongle_release_single(coder, coder->right_dongle);
}

void	wake_all_dongles(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_mutex_lock(&data->dongles[i].lock);
		pthread_cond_broadcast(&data->dongles[i].cond);
		pthread_mutex_unlock(&data->dongles[i].lock);
		i++;
	}
}
