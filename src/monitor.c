/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

static void	cx_wake_all_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		pthread_cond_broadcast(&sim->coders[i].cond);
		i++;
	}
}

static int	cx_coder_burned_out(t_sim *sim, int idx, long long now)
{
	long long	diff;

	if (sim->num_compiles_required > 0
		&& sim->coders[idx].compile_count >= sim->num_compiles_required)
		return (0);
	diff = now - sim->coders[idx].last_compile_start_ms;
	if (diff >= sim->time_to_burnout)
		return (1);
	return (0);
}

static int	cx_check_burnout(t_sim *sim)
{
	long long	now;
	int			i;

	pthread_mutex_lock(&sim->state_lock);
	if (sim->stopped)
	{
		pthread_mutex_unlock(&sim->state_lock);
		return (1);
	}
	now = cx_get_time_ms();
	i = -1;
	while (++i < sim->num_coders)
	{
		if (cx_coder_burned_out(sim, i, now))
		{
			sim->stopped = 1;
			cx_log_burnout(sim, sim->coders[i].id);
			cx_wake_all_coders(sim);
			pthread_mutex_unlock(&sim->state_lock);
			return (1);
		}
	}
	cx_dispatch_requests(sim);
	pthread_mutex_unlock(&sim->state_lock);
	return (0);
}

void	*cx_monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (!cx_is_stopped(sim))
	{
		if (cx_check_burnout(sim))
			break ;
		usleep(500);
	}
	return (NULL);
}
