/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seledka <seledka@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/09 15:30:00 by seledka           #+#    #+#             */
/*   Updated: 2026/09/09 15:30:00 by seledka          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/codexion.h"

int	main(int argc, char **argv)
{
	t_sim	sim;

	memset(&sim, 0, sizeof(t_sim));
	if (!cx_parse_args(&sim, argc, argv))
		return (1);
	if (!cx_sim_init(&sim))
	{
		cx_sim_destroy(&sim);
		write(2, "Error: Initialization failed\n", 29);
		return (1);
	}
	if (!cx_start_threads(&sim))
	{
		cx_set_stopped(&sim);
		cx_join_threads(&sim);
		cx_sim_destroy(&sim);
		write(2, "Error: Thread creation failed\n", 30);
		return (1);
	}
	cx_join_threads(&sim);
	cx_sim_destroy(&sim);
	return (0);
}
