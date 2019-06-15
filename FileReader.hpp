#ifndef FILEREADER_INCLUDED
#define FILEREADER_INCLUDED

#include <fmt/core.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string_view>
#include <optional>

class FileReader
{
public:
    FileReader(std::string_view filename)
        : ptr_(nullptr)
    {
        int fd = open(filename.data(), O_RDONLY);
        if (fd == -1) {
            throw std::runtime_error(fmt::format("Unable to open file {}: {}", filename, strerror(errno)));
        }
        fd_ = fd;

        struct stat statbuf;
        memset(&statbuf, 0, sizeof statbuf);
        if (int ret = fstat(fd, &statbuf); ret == -1) {
            throw std::runtime_error(fmt::format("Unable to stat file {}: {}", filename, strerror(errno)));
        }

        length_ = statbuf.st_size;

        if (length_ > 0U) {
            ptr_ = mmap(nullptr, length_, PROT_READ, MAP_PRIVATE, fd, 0);
            if (ptr_ == MAP_FAILED) {
                throw std::runtime_error(fmt::format("Unable to mmap file {}: {}", filename, strerror(errno)));
            }
        }
    }

    ~FileReader()
    {
        if (ptr_ && ptr_ != MAP_FAILED) {
            munmap(ptr_, length_);
        }
        if (fd_) {
            close(*fd_);
        }
    }

    std::string_view data()
    {
        return std::string_view((const char*)ptr_, length_);
    }

    std::size_t size()
    {
        return length_;
    }
private:
    std::optional<int> fd_;
    void* ptr_;
    std::size_t length_;
};

#endif
