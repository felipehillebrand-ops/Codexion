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
	dongle->available_at_ms = get_timestamp_ms(coder->data);
	if (dongle->available_at_ms > LONG_MAX - coder->data->dongle_cooldown)
		dongle->available_at_ms = LONG_MAX;
	else
		dongle->available_at_ms += coder->data->dongle_cooldown;
	pthread_mutex_unlock(&dongle->lock);
}

void	coder_release_dongles(t_coder *coder)
{
	dongle_release_single(coder, coder->left_dongle);
	if (coder->left_dongle != coder->right_dongle)
		dongle_release_single(coder, coder->right_dongle);
	wake_all_dongles(coder->data);
}

void	wake_all_dongles(t_data *data)
{
	pthread_mutex_lock(&data->request_lock);
	pthread_cond_broadcast(&data->request_cond);
	pthread_mutex_unlock(&data->request_lock);
}
