#include "codexion.h"

static int	invalid_number(const char *str)
{
	int	i;

	i = 0;
	if (!str || str[0] == '\0')
		return (1);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (2);
		i++;
	}
	return (0);
}

static long	parse_positive_long(const char *str)
{
	long	result;
	int		i;

	if (invalid_number(str))
		return (-1);
	result = 0;
	i = 0;
	while (str[i])
	{
		result = result * 10 + (str[i] - '0');
		i++;
	}
	if (result <= 0)
		return (-1);
	return (result);
}

static int	invalid_scheduler(const char *str)
{
	if (!str)
		return (1);
	if (strcmp(str, "fifo") == 0 || strcmp(str, "edf") == 0)
		return (0);
	return (2);
}

void	swap(t_waiter **a, t_waiter **b)
{
	t_waiter	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

int	parsing(char **av, t_sim *sim)
{
	sim->n_coders = (int)parse_positive_long(av[1]);
	sim->time_burnout = parse_positive_long(av[2]);
	sim->time_compile = parse_positive_long(av[3]);
	sim->time_debug = parse_positive_long(av[4]);
	sim->time_refactor = parse_positive_long(av[5]);
	sim->n_req_compiles = (int)parse_positive_long(av[6]);
	sim->dongle_cd = parse_positive_long(av[7]);
	if (sim->n_coders < 0 || sim->time_burnout < 0
		|| sim->time_compile < 0 || sim->time_debug < 0
		|| sim->time_refactor < 0 || sim->n_req_compiles < 0
		|| sim->dongle_cd < 0)
		return (1);
	if (invalid_scheduler(av[8]))
		return (2);
	if (strcmp(av[8], "fifo") == 0 || strcmp(av[8], "edf") == 0)
	{
		if (strcmp(av[8], "fifo") == 0)
			sim->scheduler = SCHEDULER_FIFO;
		if (strcmp(av[8], "edf") == 0)
			sim->scheduler = SCHEDULER_EDF;
	}
	else
		return (3);
	return (0);
}
