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

void	acquire_dongle(t_coder *coder, t_dongle *dongle)
{
	t_waiter	waiter;
	long		remaining;

	pthread_mutex_lock(&dongle->mutex);
	if (dongle->in_use == 1 || dongle->queue_size > 0)
	{
		waiter.coder_id = coder->id;
		waiter.arrived_at = get_time_ms();
		waiter.deadline = coder->last_compile + coder->sim->time_burnout;
		waiter.granted = 0;
		pthread_cond_init(&waiter.ready, NULL);
		heap_push(dongle, &waiter, coder->sim->scheduler);
		while (!waiter.granted && !sim_should_stop(coder->sim))
			pthread_cond_wait(&waiter.ready, &dongle->mutex);
		pthread_cond_destroy(&waiter.ready);
		if (!waiter.granted)
			return (pthread_mutex_unlock(&dongle->mutex), (void)0);
	}
	else
		dongle->in_use = 1;
	pthread_mutex_unlock(&dongle->mutex);
	remaining = coder->sim->dongle_cd - (get_time_ms() - dongle->released_at);
	if (remaining > 0)
		ft_msleep(remaining);
}

void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	t_waiter	*next;

	pthread_mutex_lock(&dongle->mutex);
	dongle->released_at = get_time_ms();
	next = heap_pop(dongle, coder->sim->scheduler);
	if (next != NULL)
	{
		next->granted = 1;
		pthread_cond_signal(&next->ready);
	}
	else
		dongle->in_use = 0;
	pthread_mutex_unlock(&dongle->mutex);
}
