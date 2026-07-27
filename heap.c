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

static void	heap_bubble_up(t_dongle *dongle, int idx, int scheduler)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (has_priority(dongle->queue[idx], dongle->queue[parent], scheduler))
		{
			swap(&dongle->queue[idx], &dongle->queue[parent]);
			idx = parent;
		}
		else
			return ;
	}
}

static void	heap_bubble_down(t_dongle *dongle, int idx, int scheduler)
{
	int	left;
	int	right;
	int	prio;

	while (1)
	{
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		prio = idx;
		if (left < dongle->queue_size
			&& has_priority(dongle->queue[left],
				dongle->queue[prio], scheduler))
			prio = left;
		if (right < dongle->queue_size
			&& has_priority(dongle->queue[right],
				dongle->queue[prio], scheduler))
			prio = right;
		if (prio == idx)
			return ;
		swap(&dongle->queue[idx], &dongle->queue[prio]);
		idx = prio;
	}
}

void	heap_push(t_dongle *dongle, t_waiter *waiter, int scheduler)
{
	if (dongle->queue_size >= dongle->queue_cap)
		return ;
	dongle->queue[dongle->queue_size] = waiter;
	dongle->queue_size++;
	heap_bubble_up(dongle, dongle->queue_size - 1, scheduler);
}

t_waiter	*heap_pop(t_dongle *dongle, int scheduler)
{
	t_waiter	*top;

	if (dongle->queue_size == 0)
		return (NULL);
	top = dongle->queue[0];
	dongle->queue_size--;
	dongle->queue[0] = dongle->queue[dongle->queue_size];
	if (dongle->queue_size > 0)
		heap_bubble_down(dongle, 0, scheduler);
	return (top);
}
