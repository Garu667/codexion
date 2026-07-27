#include "codexion.h"

long	get_elapsed_ms(t_sim *sim)
{
	return (get_time_ms() - sim->sim_start);
}

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
}

void	ft_msleep(long ms, t_sim *sim)
{
	long	start;

	start = get_time_ms();
	while (get_time_ms() - start < ms)
	{
		usleep(100);
		if (sim_should_stop(sim))
			return ;
	}
}

int	sim_should_stop(t_sim *sim)
{
	int	stop;

	pthread_mutex_lock(&sim->stop_mutex);
	stop = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stop);
}

int	all_coders_done(t_sim *sim)
{
	int	required;
	int	compiled;
	int	i;

	i = 0;
	compiled = 0;
	required = sim->n_req_compiles;
	while (i < sim->n_coders)
	{
		pthread_mutex_lock(&sim->coders_mutex);
		compiled = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->coders_mutex);
		if (compiled < required)
			return (0);
		i++;
	}
	return (1);
}
