#include "codexion.h"

void	log_action(t_sim *sim, int coder_id, char *action)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d %s\n", get_elapsed_ms(sim), coder_id, action);
	pthread_mutex_unlock(&sim->log_mutex);
}

static void	coder_compile(t_sim *sim, t_coder *coder)
{
	pthread_mutex_lock(&sim->coders_mutex);
	coder->last_compile = get_time_ms();
	pthread_mutex_unlock(&sim->coders_mutex);
	log_action(sim, coder->id, "is compiling");
	ft_msleep(sim->time_compile);
	pthread_mutex_lock(&sim->coders_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&sim->coders_mutex);
}

static void	coder_life(
			t_sim *sim,
			t_coder *coder,
			t_dongle *first,
			t_dongle *second
		)
{
	acquire_dongle(coder, first);
	if (sim_should_stop(sim))
		return ;
	log_action(sim, coder->id, "has taken a dongle");
	acquire_dongle(coder, second);
	if (sim_should_stop(sim))
		return ;
	log_action(sim, coder->id, "has taken a dongle");
	coder_compile(sim, coder);
	if (sim_should_stop(sim))
		return ;
	release_dongle(coder, first);
	release_dongle(coder, second);
	log_action(sim, coder->id, "is debugging");
	ft_msleep(sim->time_debug);
	if (sim_should_stop(sim))
		return ;
	log_action(sim, coder->id, "is refactoring");
	ft_msleep(sim->time_refactor);
}

void	*coder_routine(void *arg)
{
	t_sim		*sim;
	t_coder		*coder;
	t_dongle	*first;
	t_dongle	*second;

	coder = (t_coder *)arg;
	sim = coder->sim;
	while (!sim_should_stop(sim))
	{
		if (coder->left->id < coder->right->id)
		{
			first = coder->left;
			second = coder->right;
		}
		else
		{
			first = coder->right;
			second = coder->left;
		}
		coder_life(sim, coder, first, second);
	}
	return (NULL);
}
