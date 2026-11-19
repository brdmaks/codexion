/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	cx_is_stopped(t_sim *sim)
{
	int	stopped;

	pthread_mutex_lock(&sim->state_lock);
	stopped = sim->stopped;
	pthread_mutex_unlock(&sim->state_lock);
	return (stopped);
}

void	cx_set_stopped(t_sim *sim)
{
	pthread_mutex_lock(&sim->state_lock);
	sim->stopped = 1;
	pthread_mutex_unlock(&sim->state_lock);
}

void	cx_log(t_sim *sim, int coder_id, const char *msg)
{
	pthread_mutex_lock(&sim->log_lock);
	if (!cx_is_stopped(sim))
		printf("%lld %d %s\n", cx_timestamp(sim), coder_id, msg);
	pthread_mutex_unlock(&sim->log_lock);
}

void	cx_log_compile(t_sim *sim, int coder_id)
{
	cx_log(sim, coder_id, "has taken a dongle");
	cx_log(sim, coder_id, "has taken a dongle");
	cx_log(sim, coder_id, "is compiling");
}

void	cx_log_burnout(t_sim *sim, int coder_id)
{
	pthread_mutex_lock(&sim->log_lock);
	printf("%lld %d burned out\n", cx_timestamp(sim), coder_id);
	pthread_mutex_unlock(&sim->log_lock);
}
