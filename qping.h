#ifndef QPING_H
#define QPING_H

#define DEFAULT_URL "https://en.wikipedia.org/wiki/Cat"
#define BUFFER_SIZE (1024)

// Structs
struct prog_flags {
    int connect_only;
};

struct data_pkg {
    size_t size;
    void* data;
};

// Functions
int parse_args(int argc, char** argv, char* url, struct prog_flags*);

size_t write_check(void* ptr, size_t size, size_t n, void* userdata);

#endif