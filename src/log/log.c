/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 10:52:33 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/07 12:46:56 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static const char	*state_to_string(t_coder_state state)
{
	if (state == STATE_COMPILING)
		return ("is compiling");
	if (state == STATE_DEBUGGING)
		return ("is debugging");
	if (state == STATE_REFACTORING)
		return ("is refactoring");
	if (state == STATE_BURNED_OUT)
		return ("burned out");
	return ("");
}

static void	print_log_line(t_data *data, int coder_id, const char *msg)
{
	pthread_mutex_lock(&data->log_lock);
	printf("%ld %d %s\n", get_timestamp_ms(data), coder_id, msg);
	pthread_mutex_unlock(&data->log_lock);
}

void	log_dongle_taken(t_data *data, int coder_id)
{
	print_log_line(data, coder_id, "has taken a dongle");
}

void	log_state_change(t_data *data, int coder_id, t_coder_state state)
{
	print_log_line(data, coder_id, state_to_string(state));
}

void	print_debug_summary(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		fprintf(stderr, "[debug] coder %d compiled %d times\n",
			data->coders[i].id, coder_get_compiles_done(&data->coders[i]));
		i++;
	}
}
