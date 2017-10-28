#ifndef FILE_H
#define FILE_H

#include <unistd.h>

class File {
    int fd;
    char* buf;
    size_t size;
    public:
    File(std::string name) {
        int err;
        (void)err; // silence unused warning

        fd = open(name.c_str(), O_RDONLY);
        assert(fd != -1);
        struct stat s;
        err = fstat(fd, &s);
        assert(err == 0);
        size = s.st_size;

        buf = new char[size];
        ssize_t bytesRead = read(fd, buf, size);
        assert(bytesRead == size);
    }
    ~File() {
        delete[] buf;
        close(fd);
    }
    char* getPtr() { return buf; }
    size_t getSize() { return size; }
};
#endif
