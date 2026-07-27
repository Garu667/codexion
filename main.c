/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ramaroud <ramaroud@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 14:18:32 by ramaroud          #+#    #+#             */
/*   Updated: 2026/06/11 14:18:32 by ramaroud         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	cleanup_sim(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n_coders)
	{
		free(sim->dongles[i].queue);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
	free(sim->dongles);
	free(sim->coders);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->coders_mutex);
}

int	main(int ac, char **av)
{
	t_sim	sim;
	int		i;

	if (ac != 9)
	{
		fprintf(stderr,
			"Usage: %s n_coders time_burnout time_compile time_debug "
			"time_refactor n_compiles dongle_cd scheduler\n", av[0]);
		return (1);
	}
	memset(&sim, 0, sizeof(t_sim));
	if (parsing(av, &sim))
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return (2);
	}
	i = init_sim(&sim);
	if (i != 0)
		return (i);
	while (i < sim.n_coders)
	{
		pthread_join(sim.coders[i].thread, NULL);
		i++;
	}
	cleanup_sim(&sim);
}
