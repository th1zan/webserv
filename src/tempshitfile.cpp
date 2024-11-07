void Client::handleClientRequest()
{
	std::istringstream requestStream(this->_request);
	std::string method, path, version;
	requestStream >> method >> path >> version;

	// Identifier le bloc location correspondant
	Location selectedLocation = this->_server.findMatchingLocation(path);

	// Vérifier si la méthode est autorisée dans cette location
	if (!selectedLocation.isMethodAllowed(method)) {
		const char* methodNotAllowedResponse = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 23\r\n\r\n405 Method Not Allowed";
		send(this->_socket, methodNotAllowedResponse, strlen(methodNotAllowedResponse), 0);
		return;
	}

	// Vérification pour les requêtes GET, POST et DELETE
	if (method == "GET") {
		// Construire le chemin du fichier
		std::string filePath = this->_server.getRoot() + path;

		// Si "try_file" est spécifié et que le fichier n'existe pas, essaie de charger ce fichier à la place
		if (selectedLocation.hasTryFile() && !std::ifstream(filePath)) {
			filePath = this->_server.getRoot() + selectedLocation.getTryFile();
		}

		// Autoindex
		if (selectedLocation.isAutoIndexEnabled() && path == "/") {
			// Gérer l'affichage de l'autoindex ici
			// Par exemple : génère une liste de fichiers et de répertoires
		}

		// Envoie du fichier si trouvé
		std::ifstream file(filePath);
		if (file.is_open()) {
			std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			std::ostringstream response;
			response << "HTTP/1.1 200 OK\r\n";
			response << "Content-Length: " << fileContents.size() << "\r\n";
			response << "\r\n";
			response << fileContents;
			send(this->_socket, response.str().c_str(), response.str().size(), 0);
		} else {
			const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
			send(this->_socket, notFoundResponse, strlen(notFoundResponse), 0);
		}
	} else if (method == "POST") {
		// Gérer l'upload de fichiers dans la location spécifiée
		if (selectedLocation.hasUploadPath()) {
			// Gérer le téléchargement vers l'emplacement `upload_to`
		} else {
			const char* postResponse = "HTTP/1.1 200 OK\r\nContent-Length: 7\r\n\r\nPOST OK";
			send(this->_socket, postResponse, strlen(postResponse), 0);
		}
	} else if (method == "DELETE") {
		// Supprimer un fichier selon le chemin donné
		std::string filePath = this->_server.getRoot() + path;
		if (std::remove(filePath.c_str()) == 0) {
			const char* deleteResponse = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nFile Deleted";
			send(this->_socket, deleteResponse, strlen(deleteResponse), 0);
		} else {
			const char* notFoundResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: 13\r\n\r\n404 Not Found";
			send(this->_socket, notFoundResponse, strlen(notFoundResponse), 0);
		}
	} else {
		// Méthode non implémentée
		const char* notImplementedResponse = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 19\r\n\r\n501 Not Implemented";
		send(this->_socket, notImplementedResponse, strlen(notImplementedResponse), 0);
	}
}
