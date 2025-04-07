#include "ytt_generator.h"
#include <CLI/CLI.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// dumb and simple way to parse CSV
std::vector<ChatMessage> parseCSV(const std::string &filename, int timeMultiplier) {
    std::vector<ChatMessage> messages;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        std::exit(-1);
    }

    std::getline(file, line);
    if (line != "time,user_name,user_color,message") {
        std::cerr << "Error: Unexpected CSV header format.\n";
        std::exit(-1);
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string field;
        ChatMessage msg;

        std::getline(ss, field, ',');
        msg.time = std::stoi(field) * timeMultiplier;

        std::getline(ss, msg.user.name, ',');

        std::getline(ss, field, ',');
        msg.user.color = field.empty() ? getRandomColor(msg.user.name) : Color(field);

        std::getline(ss, msg.message);

        if (msg.message.size() >= 2 &&
            msg.message.front() == '"' &&
            msg.message.back() == '"') {
            msg.message = msg.message.substr(1, msg.message.size() - 2);
        }

        messages.emplace_back(std::move(msg));
    }

    return messages;
}


int main(int argc, char *argv[]) {
    CLI::App app{"Chat → YTT/SRV3 subtitle generator"};

    std::string configPath, csvPath, outputPath;
    std::string timeUnit;

    app.add_option("-c,--config", configPath, "Path to INI config file")
            ->required()
            ->check(CLI::ExistingFile);
    app.add_option("-i,--input", csvPath, "Path to chat CSV file")
            ->required()
            ->check(CLI::ExistingFile);
    app.add_option("-o,--output", outputPath, "Output file (e.g. output.srv3 or output.ytt)")
            ->required();
    app.add_option("-u,--time-unit", timeUnit, "Time unit inside CSV: “ms” or “sec”")
            ->required()
            ->check(CLI::IsMember({"ms", "sec"}, CLI::ignore_case));

    CLI11_PARSE(app, argc, argv);

    int multiplier = (timeUnit == "sec") ? 1000 : 1;

    ChatParams params;
    if (!params.loadFromFile(configPath.c_str())) {
        std::cerr << "Error: Cannot open config file: " << configPath << "\n";
        return 1;
    }

    auto chat = parseCSV(csvPath, multiplier);
    if (chat.empty()) {
        std::cerr << "Error: Failed to parse chat CSV or it's empty: " << csvPath << "\n";
        return 1;
    }

    std::string xml = generateXML(chat, params);

    std::ofstream out(outputPath);
    if (!out) {
        std::cerr << "Error: Cannot open output file: " << outputPath << "\n";
        return 1;
    }
    out << xml;
    std::cout << "Successfully wrote subtitles to: " << outputPath << "\n";
    return 0;
}
