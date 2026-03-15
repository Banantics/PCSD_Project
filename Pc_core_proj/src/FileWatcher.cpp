#include "FileWatcher.hpp"

#include <chrono>
#include <thread>
#include <utility>

FileWatcher::FileWatcher(std::string path)
    : path_(std::move(path))
{
}

bool FileWatcher::waitForFile()
{
    using namespace std::chrono_literals;

    while (true)
    {
        std::ifstream probe(path_);
        if (probe.good())
        {
            return true;
        }

        std::this_thread::sleep_for(500ms);
    }
}

bool FileWatcher::openAtBeginning()
{
    return open(std::ios::beg);
}

bool FileWatcher::openAtEnd()
{
    return open(std::ios::end);
}

FileWatcher::ReadResult FileWatcher::pollNextLine(std::string& line)
{
    const auto lineStart = file_.tellg();

    if (std::getline(file_, line))
    {
        if (file_.eof())
        {
            file_.clear();
            file_.seekg(lineStart);
            return ReadResult::NoLineAvailable;
        }

        return ReadResult::LineRead;
    }

    if (file_.eof())
    {
        file_.clear();
        return ReadResult::NoLineAvailable;
    }

    return ReadResult::Error;
}

bool FileWatcher::open(std::ios::seekdir origin)
{
    file_.close();
    file_.clear();
    file_.open(path_);
    if (!file_.is_open())
    {
        return false;
    }

    file_.seekg(0, origin);
    return true;
}
