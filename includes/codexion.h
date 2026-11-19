/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <time.h>

typedef enum e_sched
{
	CX_SCHED_FIFO,
	CX_SCHED_EDF
}	t_sched;

typedef struct s_request
{
	int			coder_id;
	int			left_dongle;
	int			right_dongle;
	long long	sequence;
	long long	deadline;
}	t_request;

typedef struct s_heap
{
	t_request	*arr;
	int			size;
	int			capacity;
	t_sched		sched;
}	t_heap;

typedef struct s_dongle
{
	int				id;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	int				is_in_use;
	int				holder_id;
	long long		available_at;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	int				left_dongle;
	int				right_dongle;
	pthread_t		thread;
	long long		last_compile_start_ms;
	int				compile_count;
	pthread_cond_t	cond;
	int				dongles_granted;
	struct s_sim	*sim;
}	t_coder;

typedef struct s_sim
{
	int				num_coders;
	long long		time_to_burnout;
	long long		time_to_compile;
	long long		time_to_debug;
	long long		time_to_refactor;
	int				num_compiles_required;
	long long		dongle_cooldown;
	t_sched			sched;

	long long		start_time_ms;
	long long		seq_counter;

	int				stopped;
	int				finished_count;

	pthread_mutex_t	state_lock;
	pthread_mutex_t	log_lock;

	t_heap			request_heap;
	t_request		*deferred_requests;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_t		monitor_thread;
}	t_sim;

/* parse.c */
int			cx_is_digit(char c);
int			cx_parse_args(t_sim *sim, int argc, char **argv);

/* time.c */
long long	cx_get_time_ms(void);
long long	cx_timestamp(t_sim *sim);
void		cx_ms_sleep(t_sim *sim, long long ms);
void		cx_timed_wait(pthread_cond_t *cond, pthread_mutex_t *mutex,
				long long ms);

/* log.c */
int			cx_is_stopped(t_sim *sim);
void		cx_set_stopped(t_sim *sim);
void		cx_log(t_sim *sim, int coder_id, const char *msg);
void		cx_log_compile(t_sim *sim, int coder_id);
void		cx_log_burnout(t_sim *sim, int coder_id);

/* heap.c */
int			cx_heap_init(t_heap *heap, int capacity, t_sched sched);
void		cx_heap_destroy(t_heap *heap);
void		cx_heap_swap(t_request *a, t_request *b);
int			cx_heap_push(t_heap *heap, t_request req);
int			cx_heap_pop(t_heap *heap, t_request *out_req);

/* heap_order.c */
int			cx_edf_tie_break(t_request a, t_request b);
int			cx_request_has_priority(t_request a, t_request b, t_sched sched);
void		cx_heap_sift_up(t_heap *heap, int idx);
void		cx_heap_sift_down(t_heap *heap, int idx);

/* dispatch.c */
int			cx_is_dongle_available(t_sim *sim, int dongle_id, long long now);
void		cx_release_dongles_lock(t_sim *sim, int d1, int d2, long long now);
void		cx_dispatch_requests(t_sim *sim);

/* coder.c */
int			cx_coder_acquire(t_coder *coder);
void		cx_coder_release(t_coder *coder);
void		*cx_coder_routine(void *arg);

/* monitor.c */
void		*cx_monitor_routine(void *arg);

/* init.c */
int			cx_sim_init(t_sim *sim);
int			cx_start_threads(t_sim *sim);

/* cleanup.c */
void		cx_join_threads(t_sim *sim);
void		cx_sim_destroy(t_sim *sim);

#endif
