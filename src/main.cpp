/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thibault <thibault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 13:55:53 by thibault          #+#    #+#             */
/*   Updated: 2024/10/07 15:00:00 by thibault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int main(int argc, char **argv)
{
	try
	{
		//Instancie un Service sw (webserver)
		Service webserv(argc, argv);
		//webserv.printServerInfo();
		webserv.setup();
		webserv.launch();
	}
	catch(const std::exception& e)
	{
		std::cerr << RED << "Error :\t" << e.what() << RESET << std::endl;
		return (EXIT_FAILURE);
	}
	
}