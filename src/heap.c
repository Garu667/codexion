/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ramaroud <ramaroud@student.42lyon.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 11:14:17 by ramaroud          #+#    #+#             */
/*   Updated: 2026/06/29 11:14:17 by ramaroud         ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	has_priority(t_waiter *a, t_waiter *b, int scheduler)
{
	if (scheduler == SCHEDULER_FIFO)
		return (a->arrived_at < b->arrived_at);
	if (a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	return (a->arrived_at < b->arrived_at);
}

t_waiter	*heap_peek(t_dongle *dongle)
{
	if (dongle->queue_size == 0)
		return (NULL);
	return (dongle->queue[0]);
}

void	heap_push(t_dongle *dongle, t_waiter *waiter, int scheduler)
{
	if (dongle->queue_size >= dongle->queue_cap)
		return ;
	dongle->queue[dongle->queue_size] = waiter;
	dongle->queue_size++;
	if (dongle->queue_size == 2
		&& has_priority(dongle->queue[1], dongle->queue[0], scheduler))
		swap(&dongle->queue[0], &dongle->queue[1]);
}

t_waiter	*heap_pop(t_dongle *dongle, int scheduler)
{
	t_waiter	*top;

	if (dongle->queue_size == 0)
		return (NULL);
	top = dongle->queue[0];
	dongle->queue_size--;
	dongle->queue[0] = dongle->queue[dongle->queue_size];
	return (top);
}
