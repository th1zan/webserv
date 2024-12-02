#include <cstdio>
#include <iostream>

int main() {
    FILE* pipe = popen("which python3", "r");
    if (!pipe) {
        std::cerr << "Failed to run command." << std::endl;
        return 1;
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        std::cout << buffer; // Print each line of the command's output
    }

    pclose(pipe); // Close the pipe
    return 0;
}