/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

long long	cx_get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (((long long)tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

long long	cx_timestamp(t_sim *sim)
{
	return (cx_get_time_ms() - sim->start_time_ms);
}

void	cx_ms_sleep(t_sim *sim, long long ms)
{
	long long	start;

	start = cx_get_time_ms();
	while (!cx_is_stopped(sim))
	{
		if (cx_get_time_ms() - start >= ms)
			break ;
		usleep(500);
	}
}

void	cx_timed_wait(pthread_cond_t *cond, pthread_mutex_t *mutex,
		long long ms)
{
	struct timespec	ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_nsec += (ms % 1000) * 1000000L;
	ts.tv_sec += (ms / 1000) + (ts.tv_nsec / 1000000000L);
	ts.tv_nsec %= 1000000000L;
	pthread_cond_timedwait(cond, mutex, &ts);
}
