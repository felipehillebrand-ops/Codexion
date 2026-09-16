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

static int	do_compile(t_coder *coder)
{
	if (!coder_begin_compile(coder))
		return (0);
	if (!sleep_ms(coder->data, coder->data->time_to_compile))
		return (0);
	coder_increment_compiles(coder);
	return (1);
}

static int	do_debug(t_coder *coder)
{
	log_state_change(coder->data, coder->id, STATE_DEBUGGING);
	return (sleep_ms(coder->data, coder->data->time_to_debug));
}

static int	do_refactor(t_coder *coder)
{
	log_state_change(coder->data, coder->id, STATE_REFACTORING);
	return (sleep_ms(coder->data, coder->data->time_to_refactor));
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	int		compiled;

	coder = (t_coder *)arg;
	if (!prepare_coder(coder)
		|| coder->data->number_of_compiles_required == 0)
		return (NULL);
	while (!is_simulation_stopped(coder->data))
	{
		if (coder_acquire_dongles(coder) != 0)
			break ;
		compiled = do_compile(coder);
		coder_release_dongles(coder);
		if (!compiled || !do_debug(coder) || !do_refactor(coder))
			break ;
	}
	return (NULL);
}
