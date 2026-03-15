#pragma once

#include <fstream>
#include <string>

class FileWatcher
{
public:
    enum class ReadResult
    {
        LineRead,
        NoLineAvailable,
        Error
    };

    explicit FileWatcher(std::string path);

    bool waitForFile();
    bool openAtBeginning();
    bool openAtEnd();
    ReadResult pollNextLine(std::string& line);

private:
    bool open(std::ios::seekdir origin);

    std::string path_;
    std::ifstream file_;
};
