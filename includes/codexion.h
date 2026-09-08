/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fjose-hi <fjose-hi@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/17 22:04:03 by fjose-hi          #+#    #+#             */
/*   Updated: 2026/09/01 03:08:44 by fjose-hi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <limits.h>

/* ---- Constants ---- */
# define ARG_COUNT		9
# define SCHED_FIFO_STR	"fifo"
# define SCHED_EDF_STR	"edf"

/* ---- Scheduler type ---- */
typedef enum e_sched_type
{
	CX_SCHED_FIFO,
	CX_SCHED_EDF
}	t_sched_type;

/* ---- Coder state ---- */
typedef enum e_coder_state
{
	STATE_IDLE,
	STATE_TAKING_DONGLE,
	STATE_COMPILING,
	STATE_DEBUGGING,
	STATE_REFACTORING,
	STATE_BURNED_OUT
}	t_coder_state;

/* ---- Forward declarations ---- */
typedef struct s_data	t_data;
typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;

/* ---- Heap node (FIFO/EDF priority queue) ---- */
typedef struct s_heap_node
{
	long		key;
	int			coder_id;
}	t_heap_node;

/* ---- Priority queue (binary heap) ---- */
typedef struct s_heap
{
	t_heap_node	*nodes;
	int			size;
	int			capacity;
}	t_heap;

/* ---- A dongle shared between two neighbouring coders ---- */
struct s_dongle
{
	int				id;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	int				is_available;
	long			available_at_ms;
	t_heap			waiting_queue;
	long			request_counter;
};

/* ---- A single coder (one pthread each) ---- */
struct s_coder
{
	int				id;
	pthread_t		thread;
	t_coder_state	state;
	int				compiles_done;
	long			last_compile_start;
	pthread_mutex_t	progress_lock;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	t_data			*data;
};

/* ---- Global simulation data ---- */
struct s_data
{
	/* parsed arguments */
	int				number_of_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				number_of_compiles_required;
	long			dongle_cooldown;
	t_sched_type	scheduler;

/* runtime state */
	struct timeval	start_time;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor_thread;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	stop_lock;
	int				simulation_stopped;
};

/* ---- Parsing functions ---- */
int		parse_args(t_data *data, char **argv);

/* ---- Init functions ---- */
int		init_data(t_data *data);

/* ---- Heap functions ---- */
void	heapify_up(t_heap *heap, int idx);
void	heapify_down(t_heap *heap, int idx);
int		heap_push(t_heap *heap, long key, int coder_id);
int		heap_pop(t_heap *heap, t_heap_node *out);

/* ---- Dongle functions ---- */
int		dongle_acquire_single(t_coder *coder, t_dongle *dongle);
void	dongle_release_single(t_coder *coder, t_dongle *dongle);
int		coder_acquire_dongles(t_coder *coder);
void	coder_release_dongles(t_coder *coder);
void	wake_all_dongles(t_data *data);

/* ---- Coder functions ---- */
void	*coder_routine(void *arg);
void	coder_set_compile_start(t_coder *coder, long ts);
long	coder_get_last_compile_start(t_coder *coder);
void	coder_increment_compiles(t_coder *coder);
int		coder_get_compiles_done(t_coder *coder);

/* ---- Log functions ---- */
void	log_dongle_taken(t_data *data, int coder_id);
void	log_state_change(t_data *data, int coder_id, t_coder_state state);

/* ---- Monitor functions ---- */
void	*monitor_routine(void *arg);

/* ---- Utils functions ---- */
void	clean_data(t_data *data);
int		is_simulation_stopped(t_data *data);
void	set_simulation_stopped(t_data *data);
void	record_start_time(t_data *data);
long	get_timestamp_ms(t_data *data);
void	ms_to_abstime(t_data *data, long target_ms, struct timespec *ts);

#endif