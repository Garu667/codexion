/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ramaroud <ramaroud@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/06 08:34:55 by ramaroud          #+#    #+#             */
/*   Updated: 2026/08/06 08:34:55 by ramaroud         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# define SCHEDULER_FIFO 1
# define SCHEDULER_EDF  2

typedef struct s_sim	t_sim;

typedef struct s_waiter
{
	int				coder_id;
	long			arrived_at;
	long			deadline;
}	t_waiter;

typedef struct s_dongle
{
	int				id;
	pthread_mutex_t	mutex;
	int				in_use;
	long			released_at;
	t_waiter		**queue;
	int				queue_size;
	int				queue_cap;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_sim			*sim;
	t_dongle		*left;
	t_dongle		*right;
	long			last_compile;
	long			deadline;
	int				compile_count;
}	t_coder;

typedef struct s_sim
{
	int				n_coders;
	long			time_burnout;
	long			time_compile;
	long			time_debug;
	long			time_refactor;
	int				n_req_compiles;
	long			dongle_cd;
	int				scheduler;
	t_dongle		*dongles;
	t_coder			*coders;
	long			sim_start;
	int				stop;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	log_mutex;
	pthread_t		monitor;
	pthread_mutex_t	coders_mutex;
}	t_sim;

/*		parsing.c		*/
void		swap(t_waiter **a, t_waiter **b);
int			parsing(char **av, t_sim *sim);
/*		utils.c		*/
long		get_time_ms(void);
void		ft_msleep(long ms, t_sim *sim);
long		get_elapsed_ms(t_sim *sim);
int			all_coders_done(t_sim *sim);
int			sim_should_stop(t_sim *sim);
/*		heap.c		*/
t_waiter	*heap_pop(t_dongle *dongle);
t_waiter	*heap_peek(t_dongle *dongle);
void		heap_push(t_dongle *dongle, t_waiter *waiter, int scheduler);
/*		init.c		*/
void		*monitor_routine(void *arg);
int			init_sim(t_sim *sim);
/*		dongle.c		*/
void		acquire_dongle(t_coder *coder, t_dongle *dongle);
void		release_dongle(t_dongle *dongle);
/*		coders.c		*/
void		log_action(t_sim *sim, int coder_id, char *action);
void		*coder_routine(void *arg);
long		coder_status(t_sim *sim, int i, int *done);
/*		main.c		*/
void		cleanup_sim(t_sim *sim, int i);

#endif
