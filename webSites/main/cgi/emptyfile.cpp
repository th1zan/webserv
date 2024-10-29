// EasterEgg - ExamRank05 training (29.04)

//Parsing JSON training


#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

std::string trim(const std::string& str){
	size_t first = str.find_first_not_of(' ');
	if(first == std::string::npos)
		return "";
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last-first + 1));

}

bool parseJSON(const std::string& filename, std::map<std::string, std::string>& jsonMap){

	std::ifstream file((filename.c_str()));
	
	if(!file.is_open()){
		std::cerr << "error opening file" << std::endl;
		return false;
	}

	std::string line, key, value;
	bool inObject = false;

	while(getline(file, line)){
		
		line = trim(line);
		
		if(line == "{")
		{
			inObject = true;
			continue;
		}
		else if(line == "}"){
			inObject = false;
			break;
		}
		size_t colonPos = line.find(':');
		if(colonPos == std::string::npos || !inObject){
			std::cerr << "errreur" << std::endl;
			return false;
		}
	
		key = trim(line.substr(0, colonPos));
		value = trim (line.substr(colonPos + 1));

		if(key[0] == '"' && key[key.size() - 1] == '"')
			key = key.substr(1, key.size() - 2);
		if(value[0] == '"' && value[key.size() - 1] == '"')
			value = value.substr(1, value.size() - 2);
		
		jsonMap[key] = value;

		if(inObject) {
			std::cerr << "errreur" << std::endl;
			return false;
		}	

		return true;
	}

	
}
