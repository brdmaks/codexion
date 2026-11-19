/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dispatch.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	cx_is_dongle_available(t_sim *sim, int dongle_id, long long now)
{
	int	avail;

	pthread_mutex_lock(&sim->dongles[dongle_id].mutex);
	avail = (!sim->dongles[dongle_id].is_in_use
			&& now >= sim->dongles[dongle_id].available_at);
	pthread_mutex_unlock(&sim->dongles[dongle_id].mutex);
	return (avail);
}

static void	cx_commit_grant(t_sim *sim, t_request *req, long long now)
{
	int	first;
	int	second;

	first = req->left_dongle;
	second = req->right_dongle;
	if (first > second)
	{
		first = req->right_dongle;
		second = req->left_dongle;
	}
	pthread_mutex_lock(&sim->dongles[first].mutex);
	pthread_mutex_lock(&sim->dongles[second].mutex);
	sim->dongles[req->left_dongle].is_in_use = 1;
	sim->dongles[req->left_dongle].holder_id = req->coder_id;
	sim->dongles[req->right_dongle].is_in_use = 1;
	sim->dongles[req->right_dongle].holder_id = req->coder_id;
	pthread_mutex_unlock(&sim->dongles[second].mutex);
	pthread_mutex_unlock(&sim->dongles[first].mutex);
	sim->coders[req->coder_id - 1].dongles_granted = 1;
	sim->coders[req->coder_id - 1].last_compile_start_ms = now;
	pthread_cond_signal(&sim->coders[req->coder_id - 1].cond);
}

void	cx_release_dongles_lock(t_sim *sim, int d1, int d2, long long now)
{
	int	first;
	int	second;

	first = d1;
	second = d2;
	if (first > second)
	{
		first = d2;
		second = d1;
	}
	pthread_mutex_lock(&sim->dongles[first].mutex);
	pthread_mutex_lock(&sim->dongles[second].mutex);
	sim->dongles[d1].is_in_use = 0;
	sim->dongles[d1].holder_id = 0;
	sim->dongles[d1].available_at = now + sim->dongle_cooldown;
	sim->dongles[d2].is_in_use = 0;
	sim->dongles[d2].holder_id = 0;
	sim->dongles[d2].available_at = now + sim->dongle_cooldown;
	pthread_mutex_unlock(&sim->dongles[second].mutex);
	pthread_mutex_unlock(&sim->dongles[first].mutex);
}

void	cx_dispatch_requests(t_sim *sim)
{
	t_request	req;
	long long	now;
	int			def_count;
	int			i;

	if (sim->stopped || sim->num_coders <= 1)
		return ;
	now = cx_get_time_ms();
	def_count = 0;
	while (cx_heap_pop(&sim->request_heap, &req))
	{
		if (sim->coders[req.coder_id - 1].dongles_granted)
			continue ;
		if (cx_is_dongle_available(sim, req.left_dongle, now)
			&& cx_is_dongle_available(sim, req.right_dongle, now))
			cx_commit_grant(sim, &req, now);
		else
			sim->deferred_requests[def_count++] = req;
	}
	i = 0;
	while (i < def_count)
	{
		cx_heap_push(&sim->request_heap, sim->deferred_requests[i]);
		i++;
	}
}
