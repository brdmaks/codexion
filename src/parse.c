/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	cx_is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static int	cx_parse_pos_long(const char *str, long long *val)
{
	int			i;
	long long	res;

	if (!str || !str[0])
		return (0);
	i = 0;
	res = 0;
	while (str[i])
	{
		if (!cx_is_digit(str[i]))
			return (0);
		res = res * 10 + (str[i] - '0');
		if (res > 2147483647)
			return (0);
		i++;
	}
	*val = res;
	return (1);
}

static int	cx_parse_scheduler(t_sim *sim, const char *str)
{
	if (!str)
		return (0);
	if (strcmp(str, "fifo") == 0)
	{
		sim->sched = CX_SCHED_FIFO;
		return (1);
	}
	if (strcmp(str, "edf") == 0)
	{
		sim->sched = CX_SCHED_EDF;
		return (1);
	}
	return (0);
}

static int	cx_parse_nums(t_sim *sim, char **argv)
{
	long long	tmp;

	if (!cx_parse_pos_long(argv[1], &tmp) || tmp < 1)
		return (0);
	sim->num_coders = (int)tmp;
	if (!cx_parse_pos_long(argv[2], &sim->time_to_burnout)
		|| sim->time_to_burnout < 1)
		return (0);
	if (!cx_parse_pos_long(argv[3], &sim->time_to_compile))
		return (0);
	if (!cx_parse_pos_long(argv[4], &sim->time_to_debug))
		return (0);
	if (!cx_parse_pos_long(argv[5], &sim->time_to_refactor))
		return (0);
	if (!cx_parse_pos_long(argv[6], &tmp))
		return (0);
	sim->num_compiles_required = (int)tmp;
	if (!cx_parse_pos_long(argv[7], &sim->dongle_cooldown))
		return (0);
	return (1);
}

int	cx_parse_args(t_sim *sim, int argc, char **argv)
{
	if (argc != 9)
	{
		write(2, "Error: Invalid number of arguments\n", 35);
		return (0);
	}
	if (!cx_parse_nums(sim, argv))
	{
		write(2, "Error: Invalid numeric argument\n", 32);
		return (0);
	}
	if (!cx_parse_scheduler(sim, argv[8]))
	{
		write(2, "Error: Scheduler must be 'fifo' or 'edf'\n", 41);
		return (0);
	}
	return (1);
}
