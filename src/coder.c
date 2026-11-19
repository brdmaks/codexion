/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

static int	cx_coder_acquire_one(t_coder *coder, t_sim *sim)
{
	pthread_mutex_lock(&sim->dongles[0].mutex);
	sim->dongles[0].is_in_use = 1;
	sim->dongles[0].holder_id = 1;
	pthread_mutex_unlock(&sim->dongles[0].mutex);
	pthread_mutex_unlock(&sim->state_lock);
	cx_log(sim, 1, "has taken a dongle");
	pthread_mutex_lock(&sim->state_lock);
	while (!sim->stopped)
		pthread_cond_wait(&coder->cond, &sim->state_lock);
	pthread_mutex_unlock(&sim->state_lock);
	return (0);
}

static void	cx_push_request(t_coder *coder, t_sim *sim)
{
	t_request	req;

	req.coder_id = coder->id;
	req.left_dongle = coder->left_dongle;
	req.right_dongle = coder->right_dongle;
	req.sequence = ++sim->seq_counter;
	req.deadline = coder->last_compile_start_ms + sim->time_to_burnout;
	coder->dongles_granted = 0;
	cx_heap_push(&sim->request_heap, req);
	cx_dispatch_requests(sim);
}

int	cx_coder_acquire(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	if (sim->stopped)
	{
		pthread_mutex_unlock(&sim->state_lock);
		return (0);
	}
	if (sim->num_coders == 1)
		return (cx_coder_acquire_one(coder, sim));
	cx_push_request(coder, sim);
	while (!coder->dongles_granted && !sim->stopped)
	{
		cx_timed_wait(&coder->cond, &sim->state_lock, 5);
		cx_dispatch_requests(sim);
	}
	if (sim->stopped)
	{
		pthread_mutex_unlock(&sim->state_lock);
		return (0);
	}
	pthread_mutex_unlock(&sim->state_lock);
	cx_log_compile(sim, coder->id);
	return (1);
}

void	cx_coder_release(t_coder *coder)
{
	t_sim		*sim;
	long long	now;

	sim = coder->sim;
	pthread_mutex_lock(&sim->state_lock);
	now = cx_get_time_ms();
	cx_release_dongles_lock(sim, coder->left_dongle, coder->right_dongle, now);
	coder->compile_count++;
	if (sim->num_compiles_required > 0
		&& coder->compile_count == sim->num_compiles_required
		&& ++sim->finished_count == sim->num_coders)
		sim->stopped = 1;
	cx_dispatch_requests(sim);
	pthread_mutex_unlock(&sim->state_lock);
	cx_log(sim, coder->id, "is debugging");
	cx_ms_sleep(sim, sim->time_to_debug);
	cx_log(sim, coder->id, "is refactoring");
	cx_ms_sleep(sim, sim->time_to_refactor);
}

void	*cx_coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	if (sim->num_compiles_required == 0)
		return (NULL);
	while (!cx_is_stopped(sim))
	{
		if (!cx_coder_acquire(coder))
			break ;
		cx_ms_sleep(sim, sim->time_to_compile);
		cx_coder_release(coder);
		if (sim->num_compiles_required > 0
			&& coder->compile_count >= sim->num_compiles_required)
			break ;
	}
	return (NULL);
}
