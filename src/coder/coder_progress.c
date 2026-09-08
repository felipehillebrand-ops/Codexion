/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_progress.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/03 10:47:49 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/07 12:39:51 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	coder_set_compile_start(t_coder *coder, long ts)
{
	pthread_mutex_lock(&coder->progress_lock);
	coder->last_compile_start = ts;
	pthread_mutex_unlock(&coder->progress_lock);
}

long	coder_get_last_compile_start(t_coder *coder)
{
	long	value;

	pthread_mutex_lock(&coder->progress_lock);
	value = coder->last_compile_start;
	pthread_mutex_unlock(&coder->progress_lock);
	return (value);
}

void	coder_increment_compiles(t_coder *coder)
{
	pthread_mutex_lock(&coder->progress_lock);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->progress_lock);
}

int	coder_get_compiles_done(t_coder *coder)
{
	int	value;

	pthread_mutex_lock(&coder->progress_lock);
	value = coder->compiles_done;
	pthread_mutex_unlock(&coder->progress_lock);
	return (value);
}
