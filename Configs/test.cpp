#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <unordered_set>

// Maps to store starting and ending task names
std::map<std::string, std::unordered_set<std::string>> starting_task_names;
std::map<std::string, std::unordered_set<std::string>> ending_task_names;

// Function to trim whitespace from a string
std::string trim(const std::string& str) {
    const char* whitespace = " \t\n\r";
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

// Function to parse the configuration file
void parseConfigFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }

    std::string line;
    std::string current_model;
    bool is_starting_task = true;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }

        if (line[0] == '[' && line.back() == ']') {
            // New model section
            current_model = line.substr(1, line.size() - 2);
            starting_task_names[current_model] = {};
            ending_task_names[current_model] = {};
            is_starting_task = true; // Reset for the new model
        } else if (!current_model.empty()) {
            if (is_starting_task) {
                starting_task_names[current_model].insert(line);
            } else {
                ending_task_names[current_model].insert(line);
            }
            is_starting_task = !is_starting_task; // Alternate between starting and ending tasks
        }
    }

    file.close();
}

int main() {
    parseConfigFile("Alex.conf");

    // Print the parsed starting and ending task names
    for (const auto& [model, tasks] : starting_task_names) {
        std::cout << "\n\nModel: " << model << std::endl;
        std::cout << "Starting Tasks: \n";
        for (const auto& task : tasks) {
            std::cout << task << "\n";
        }
        std::cout << std::endl;

        std::cout << "Ending Tasks: \n";
        for (const auto& task : ending_task_names[model]) {
            std::cout << task << "\n";
        }
        std::cout << std::endl;
    }

    return 0;
}

