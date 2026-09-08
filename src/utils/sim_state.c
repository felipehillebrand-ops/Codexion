/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_state.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/01 10:32:17 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/07 11:35:29 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_simulation_stopped(t_data *data)
{
	int	stopped;

	pthread_mutex_lock(&data->stop_lock);
	stopped = data->simulation_stopped;
	pthread_mutex_unlock(&data->stop_lock);
	return (stopped);
}

void	set_simulation_stopped(t_data *data)
{
	pthread_mutex_lock(&data->stop_lock);
	data->simulation_stopped = 1;
	pthread_mutex_unlock(&data->stop_lock);
}
