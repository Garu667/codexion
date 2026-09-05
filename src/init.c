/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ramaroud <ramaroud@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 16:06:15 by ramaroud          #+#    #+#             */
/*   Updated: 2026/07/27 16:06:15 by ramaroud         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	stop_simulation(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	long	time;
	int		i;

	time = 0;
	sim = (t_sim *)arg;
	while (!sim_should_stop(sim))
	{
		i = -1;
		usleep(100);
		while (++i < sim->n_coders)
		{
			pthread_mutex_lock(&sim->coders_mutex);
			time = get_time_ms() - sim->coders[i].last_compile;
			pthread_mutex_unlock(&sim->coders_mutex);
			if (time > sim->time_burnout)
			{
				log_action(sim, sim->coders[i].id, "burned out");
				return (stop_simulation(sim), NULL);
			}
		}
		if (all_coders_done(sim))
			stop_simulation(sim);
	}
	return (NULL);
}

static int	init_dongles(t_dongle *dongles, int n_dongle)
{
	int	i;

	i = 0;
	while (i < n_dongle)
	{
		dongles[i].queue = malloc(n_dongle * sizeof(t_waiter *));
		if (dongles[i].queue == NULL)
		{
			while (--i >= 0)
				free(dongles[i].queue);
			return (-1);
		}
		dongles[i].queue_size = 0;
		dongles[i].queue_cap = n_dongle;
		dongles[i].id = i + 1;
		pthread_mutex_init(&dongles[i].mutex, NULL);
		dongles[i].in_use = 0;
		dongles[i].released_at = 0;
		i++;
	}
	return (0);
}

static void	init_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].sim = sim;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1) % sim->n_coders];
		sim->coders[i].last_compile = get_time_ms();
		sim->coders[i].compile_count = 0;
		i++;
	}
}

int	init_sim(t_sim *sim)
{
	int	i;

	i = -1;
	sim->coders = malloc(sim->n_coders * sizeof(t_coder));
	if (!sim->coders)
		return (3);
	sim->dongles = malloc(sim->n_coders * sizeof(t_dongle));
	if (!sim->dongles)
		return (free(sim->coders), 4);
	if (init_dongles(sim->dongles, sim->n_coders) != 0)
		return (free(sim->coders), free(sim->dongles), 5);
	init_coders(sim);
	pthread_mutex_init(&sim->stop_mutex, NULL);
	pthread_mutex_init(&sim->log_mutex, NULL);
	pthread_mutex_init(&sim->coders_mutex, NULL);
	sim->sim_start = get_time_ms();
	pthread_create(&sim->monitor, NULL, monitor_routine, sim);
	while (++i < sim->n_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (cleanup_sim(sim, i), -9);
	}
	pthread_join(sim->monitor, NULL);
	return (0);
}
