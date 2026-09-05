/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ramaroud <ramaroud@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:03:45 by ramaroud          #+#    #+#             */
/*   Updated: 2026/07/27 17:03:45 by ramaroud         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	wake_all_waiters(t_sim *sim)
{
	int	i;
	int	j;

	i = 0;
	while (i < sim->n_coders)
	{
		j = 0;
		pthread_mutex_lock(&sim->dongles[i].mutex);
		while (j < sim->dongles[i].queue_size)
		{
			pthread_cond_signal(&sim->dongles[i].queue[j]->ready);
			j++;
		}
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static int	dongle_ready(t_dongle *dongle, t_waiter *waiter, long dongle_cd)
{
	if (dongle->in_use)
		return (0);
	if (get_time_ms() - dongle->released_at < dongle_cd)
		return (0);
	return (heap_peek(dongle) == waiter);
}

void	acquire_dongle(t_coder *coder, t_dongle *dongle)
{
	t_waiter	waiter;

	waiter.coder_id = coder->id;
	waiter.arrived_at = get_time_ms();
	waiter.deadline = coder->last_compile + coder->sim->time_burnout;
	pthread_mutex_lock(&dongle->mutex);
	heap_push(dongle, &waiter, coder->sim->scheduler);
	while (!dongle_ready(dongle, &waiter, coder->sim->dongle_cd)
		&& !sim_should_stop(coder->sim))
	{
		pthread_mutex_unlock(&dongle->mutex);
		usleep(500);
		pthread_mutex_lock(&dongle->mutex);
	}
	if (dongle_ready(dongle, &waiter, coder->sim->dongle_cd)
		&& !sim_should_stop(coder->sim))
	{
		dongle->in_use = 1;
		heap_pop(dongle);
	}
	else if (dongle->queue_size == 2)
		dongle->queue_size--;
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	(void)coder;
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use = 0;
	dongle->released_at = get_time_ms();
	pthread_mutex_unlock(&dongle->mutex);
}
