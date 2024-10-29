/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsanglar <tsanglar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 13:55:53 by thibault          #+#    #+#             */
/*   Updated: 2024/10/29 17:28:31 by tsanglar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int main(int argc, char **argv)
{
	try
	{
		//Instancie un Service sw (webserver)
		//DEBUG
		std::cout << " !!! don't forget to change the path 'root /PATH_TO/..../webserv/webSites/main;' in the 'default.conf' file " << std::endl;
			
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
