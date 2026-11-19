/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

static int	cx_init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = (t_dongle *)malloc(sizeof(t_dongle) * sim->num_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->num_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].is_in_use = 0;
		sim->dongles[i].holder_id = 0;
		sim->dongles[i].available_at = 0;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
		i++;
	}
	return (1);
}

static int	cx_init_coders(t_sim *sim)
{
	int	i;

	sim->coders = (t_coder *)malloc(sizeof(t_coder) * sim->num_coders);
	if (!sim->coders)
		return (0);
	i = 0;
	while (i < sim->num_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].left_dongle = i;
		if (sim->num_coders > 1)
			sim->coders[i].right_dongle = (i + 1) % sim->num_coders;
		else
			sim->coders[i].right_dongle = -1;
		sim->coders[i].last_compile_start_ms = sim->start_time_ms;
		sim->coders[i].compile_count = 0;
		sim->coders[i].dongles_granted = 0;
		sim->coders[i].sim = sim;
		pthread_cond_init(&sim->coders[i].cond, NULL);
		i++;
	}
	return (1);
}

int	cx_sim_init(t_sim *sim)
{
	sim->stopped = 0;
	sim->finished_count = 0;
	sim->seq_counter = 0;
	pthread_mutex_init(&sim->state_lock, NULL);
	pthread_mutex_init(&sim->log_lock, NULL);
	sim->start_time_ms = cx_get_time_ms();
	sim->deferred_requests = (t_request *)malloc(sizeof(t_request)
			* sim->num_coders);
	if (!sim->deferred_requests)
		return (0);
	if (!cx_heap_init(&sim->request_heap, sim->num_coders + 10, sim->sched))
		return (0);
	if (!cx_init_dongles(sim) || !cx_init_coders(sim))
		return (0);
	return (1);
}

int	cx_start_threads(t_sim *sim)
{
	int	i;

	if (sim->num_compiles_required == 0)
	{
		sim->stopped = 1;
		return (1);
	}
	sim->start_time_ms = cx_get_time_ms();
	i = 0;
	while (i < sim->num_coders)
	{
		sim->coders[i].last_compile_start_ms = sim->start_time_ms;
		if (pthread_create(&sim->coders[i].thread, NULL,
				cx_coder_routine, &sim->coders[i]) != 0)
			return (0);
		i++;
	}
	if (pthread_create(&sim->monitor_thread, NULL,
			cx_monitor_routine, sim) != 0)
		return (0);
	return (1);
}
