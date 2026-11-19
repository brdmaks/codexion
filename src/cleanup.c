/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

void	cx_join_threads(t_sim *sim)
{
	int	i;

	if (sim->num_compiles_required == 0)
		return ;
	pthread_join(sim->monitor_thread, NULL);
	i = 0;
	while (i < sim->num_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

void	cx_sim_destroy(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		if (sim->dongles)
		{
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			pthread_cond_destroy(&sim->dongles[i].cond);
		}
		if (sim->coders)
			pthread_cond_destroy(&sim->coders[i].cond);
		i++;
	}
	pthread_mutex_destroy(&sim->state_lock);
	pthread_mutex_destroy(&sim->log_lock);
	cx_heap_destroy(&sim->request_heap);
	if (sim->dongles)
		free(sim->dongles);
	if (sim->coders)
		free(sim->coders);
	if (sim->deferred_requests)
		free(sim->deferred_requests);
}
