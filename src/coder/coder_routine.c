/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 10:58:32 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/07 13:04:38 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	do_compile(t_coder *coder)
{
	if (is_simulation_stopped(coder->data))
		return ;
	coder_set_compile_start(coder, get_timestamp_ms(coder->data));
	log_state_change(coder->data, coder->id, STATE_COMPILING);
	usleep(coder->data->time_to_compile * 1000);
	coder_increment_compiles(coder);
}

static void	do_debug(t_coder *coder)
{
	log_state_change(coder->data, coder->id, STATE_DEBUGGING);
	usleep(coder->data->time_to_debug * 1000);
}

static void	do_refactor(t_coder *coder)
{
	log_state_change(coder->data, coder->id, STATE_REFACTORING);
	usleep(coder->data->time_to_refactor * 1000);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!is_simulation_stopped(coder->data)
		&& coder_get_compiles_done(coder)
		< coder->data->number_of_compiles_required)
	{
		if (coder_acquire_dongles(coder) != 0)
			break ;
		do_compile(coder);
		coder_release_dongles(coder);
		if (is_simulation_stopped(coder->data))
			break ;
		do_debug(coder);
		if (is_simulation_stopped(coder->data))
			break ;
		do_refactor(coder);
	}
	return (NULL);
}
