#include "LogMonitorApp.hpp"

#include <chrono>
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "CsvParser.hpp"
#include "FileWatcher.hpp"
#include "SerialOutputPrinter.hpp"

namespace
{
namespace fs = std::filesystem;

using namespace std::chrono_literals;

enum class WatchMode
{
    File,
    Directory
};

struct ProgramOptions
{
    WatchMode mode;
    std::string path;
};

std::optional<std::string> detectDefaultLogsDirectory(char* executableArgument)
{
    std::error_code error;
    const fs::path currentPath = fs::current_path(error);
    const fs::path executablePath = fs::absolute(executableArgument, error);
    const fs::path executableDirectory = executablePath.parent_path();

    for (const auto& candidate : {
        currentPath / ".." / ".." / "stm32_proj" / "logs",
        currentPath / ".." / "stm32_proj" / "logs",
        currentPath / "stm32_proj" / "logs",
        executableDirectory / ".." / ".." / ".." / "stm32_proj" / "logs",
        executableDirectory / ".." / ".." / "stm32_proj" / "logs",
        executableDirectory / ".." / "stm32_proj" / "logs"
    })
    {
        error.clear();
        if (fs::exists(candidate, error) && fs::is_directory(candidate, error))
        {
            return candidate.lexically_normal().string();
        }
    }

    return std::nullopt;
}

ProgramOptions parseProgramOptions(int argc, char* argv[])
{
    const auto joinRemainingArguments = [&](int startIndex) {
        std::string result;
        for (int index = startIndex; index < argc; ++index)
        {
            if (!result.empty())
            {
                result += ' ';
            }

            result += argv[index];
        }

        return result;
    };

    if (argc >= 3 && std::string(argv[1]) == "--file")
    {
        return {WatchMode::File, joinRemainingArguments(2)};
    }

    if (argc >= 3 && std::string(argv[1]) == "--dir")
    {
        return {WatchMode::Directory, joinRemainingArguments(2)};
    }

    if (const auto detectedLogsDirectory = detectDefaultLogsDirectory(argv[0]); detectedLogsDirectory.has_value())
    {
        return {WatchMode::Directory, *detectedLogsDirectory};
    }

    return {WatchMode::File, "serial_output.csv"};
}

std::optional<std::string> findLatestLogFile(const std::string& directoryPath)
{
    std::error_code error;
    if (!fs::exists(directoryPath, error) || !fs::is_directory(directoryPath, error))
    {
        return std::nullopt;
    }

    std::optional<fs::path> latestPath;
    for (const auto& entry : fs::directory_iterator(directoryPath, error))
    {
        if (error)
        {
            return std::nullopt;
        }

        if (!entry.is_regular_file(error) || error)
        {
            continue;
        }

        const fs::path path = entry.path();
        if (path.extension() != ".log")
        {
            continue;
        }

        if (!latestPath.has_value() || path.filename().string() > latestPath->filename().string())
        {
            latestPath = path;
        }
    }

    if (!latestPath.has_value())
    {
        return std::nullopt;
    }

    return latestPath->string();
}

int watchFile(const std::string& filePath, const CsvParser& parser, const SerialOutputPrinter& printer)
{
    std::cout << "Watching file: " << filePath << std::endl;
    std::cout << "Waiting for data..." << std::endl;

    FileWatcher watcher(filePath);
    if (!watcher.waitForFile())
    {
        std::cerr << "Could not find file: " << filePath << '\n';
        return 1;
    }

    if (!watcher.openAtEnd())
    {
        std::cerr << "Could not open file: " << filePath << '\n';
        return 1;
    }

    std::string line;
    while (true)
    {
        switch (watcher.pollNextLine(line))
        {
        case FileWatcher::ReadResult::LineRead:
        {
            const auto sample = parser.parseLine(line);
            if (sample.has_value())
            {
                printer.print(*sample);
            }

            break;
        }
        case FileWatcher::ReadResult::NoLineAvailable:
            std::this_thread::sleep_for(100ms);
            break;
        case FileWatcher::ReadResult::Error:
            std::cerr << "Error while reading file: " << filePath << '\n';
            return 1;
        }
    }
}

int watchDirectory(const std::string& directoryPath, const CsvParser& parser, const SerialOutputPrinter& printer)
{
    std::error_code error;
    if (!fs::exists(directoryPath, error) || !fs::is_directory(directoryPath, error))
    {
        std::cerr << "Directory does not exist: " << directoryPath << '\n';
        return 1;
    }

    std::cout << "Watching directory: " << directoryPath << std::endl;
    std::cout << "Waiting for log files..." << std::endl;

    std::string currentFilePath;
    while (true)
    {
        if (const auto latestFile = findLatestLogFile(directoryPath); latestFile.has_value())
        {
            currentFilePath = *latestFile;
            break;
        }

        std::this_thread::sleep_for(500ms);
    }

    std::cout << "Following latest log: " << currentFilePath << std::endl;

    FileWatcher watcher(currentFilePath);
    if (!watcher.waitForFile() || !watcher.openAtEnd())
    {
        std::cerr << "Could not open file: " << currentFilePath << '\n';
        return 1;
    }

    std::string line;
    while (true)
    {
        if (const auto latestFilePath = findLatestLogFile(directoryPath);
            latestFilePath.has_value() && *latestFilePath != currentFilePath)
        {
            currentFilePath = *latestFilePath;
            std::cout << "Switching to new log file: " << currentFilePath << std::endl;

            watcher = FileWatcher(currentFilePath);
            if (!watcher.waitForFile() || !watcher.openAtBeginning())
            {
                std::cerr << "Could not open file: " << currentFilePath << '\n';
                return 1;
            }
        }

        switch (watcher.pollNextLine(line))
        {
        case FileWatcher::ReadResult::LineRead:
        {
            const auto sample = parser.parseLine(line);
            if (sample.has_value())
            {
                printer.print(*sample);
            }

            break;
        }
        case FileWatcher::ReadResult::NoLineAvailable:
            std::this_thread::sleep_for(100ms);
            break;
        case FileWatcher::ReadResult::Error:
            std::cerr << "Error while reading file: " << currentFilePath << '\n';
            return 1;
        }
    }
}
} // namespace

int LogMonitorApp::run(int argc, char* argv[]) const
{
    const ProgramOptions options = parseProgramOptions(argc, argv);

    CsvParser parser;
    SerialOutputPrinter printer;

    if (options.mode == WatchMode::Directory)
    {
        return watchDirectory(options.path, parser, printer);
    }

    return watchFile(options.path, parser, printer);
}
