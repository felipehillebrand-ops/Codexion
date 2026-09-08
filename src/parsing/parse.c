/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 21:35:04 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/01 03:11:56 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_all_digits(const char *str)
{
	int	i;

	i = 0;
	if (str[0] == '\0')
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	parse_positive_long(const char *str, long *out)
{
	long	result;
	int		i;

	if (!is_all_digits(str))
		return (-1);
	result = 0;
	i = 0;
	while (str[i])
	{
		if (result > (LONG_MAX - (str[i] - '0')) / 10)
			return (-1);
		result = result * 10 + (str[i] - '0');
		i++;
	}
	*out = result;
	return (0);
}

static int	is_valid_scheduler(const char *str, t_sched_type *out)
{
	if (strcmp(str, SCHED_FIFO_STR) == 0)
	{
		*out = CX_SCHED_FIFO;
		return (0);
	}
	if (strcmp(str, SCHED_EDF_STR) == 0)
	{
		*out = CX_SCHED_EDF;
		return (0);
	}
	return (-1);
}

int	parse_args(t_data *data, char **argv)
{
	long	tmp;

	if (parse_positive_long(argv[1], &tmp) != 0 || tmp == 0)
		return (fprintf(stderr, "Error: invalid number_of_coders\n"), -1);
	data->number_of_coders = (int)tmp;
	if (parse_positive_long(argv[2], &data->time_to_burnout) != 0)
		return (fprintf(stderr, "Error: invalid time_to_burnout\n"), -1);
	if (parse_positive_long(argv[3], &data->time_to_compile) != 0)
		return (fprintf(stderr, "Error: invalid time_to_compile\n"), -1);
	if (parse_positive_long(argv[4], &data->time_to_debug) != 0)
		return (fprintf(stderr, "Error: invalid time_to_debug\n"), -1);
	if (parse_positive_long(argv[5], &data->time_to_refactor) != 0)
		return (fprintf(stderr, "Error: invalid time_to_refactor\n"), -1);
	if (parse_positive_long(argv[6], &tmp) != 0)
		return (fprintf(stderr, "Error: invalid n_compiles_required\n"), -1);
	data->number_of_compiles_required = (int)tmp;
	if (parse_positive_long(argv[7], &data->dongle_cooldown) != 0)
		return (fprintf(stderr, "Error: invalid dongle_cooldown\n"), -1);
	if (is_valid_scheduler(argv[8], &data->scheduler) != 0)
		return (fprintf(stderr, "Error: scheduler must be 'fifo' or 'edf'\n"), -1);
	return (0);
}
