/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 21:06:09 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/01 03:09:52 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	print_usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s number_of_coders time_to_burnout ", prog_name);
	fprintf(stderr, "time_to_compile time_to_debug time_to_refactor ");
	fprintf(stderr, "number_of_compiles_required dongle_cooldown scheduler\n");
}

static void	join_threads(t_data *data, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(data->coders[i].thread, NULL);
		i++;
	}
	pthread_join(data->monitor_thread, NULL);
}

static int	run_simulation(t_data *data)
{
	int	i;

	if (pthread_create(&data->monitor_thread, NULL,
			monitor_routine, data) != 0)
		return (-1);
	i = 0;
	while (i < data->number_of_coders)
	{
		if (pthread_create(&data->coders[i].thread, NULL,
				coder_routine, &data->coders[i]) != 0)
			break ;
		i++;
	}
	if (i == data->number_of_coders)
		start_simulation(data);
	else
		set_simulation_stopped(data);
	join_threads(data, i);
	return (!data->simulation_started);
}

int	main(int argc, char **argv)
{
	t_data	data;
	int		status;

	memset(&data, 0, sizeof(data));
	if (argc != ARG_COUNT)
		return (print_usage(argv[0]), 1);
	if (parse_args(&data, argv) != 0)
		return (1);
	if (init_data(&data) != 0)
	{
		clean_data(&data);
		return (fprintf(stderr, "Error: initialization failed\n"), 1);
	}
	status = run_simulation(&data);
	print_debug_summary(&data);
	clean_data(&data);
	if (status != 0)
		return (fprintf(stderr, "Error: thread creation failed\n"), 1);
	return (0);
}
