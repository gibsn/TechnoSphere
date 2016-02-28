#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>


enum class AllocErrorType
{
    InvalidFree,
    NoMemory,
};


class AllocError: std::runtime_error
{
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};


class Allocator;


class Pointer
{
    std::shared_ptr<void *> ptr;
    std::shared_ptr<size_t> size;

public:
    Pointer();
    Pointer(void **, size_t);

    void *get() const { return *ptr; }
    size_t getSize() const { return *size; }
    void setPtr(void *p) {*ptr = p; }
    void setSize(size_t s) { *size = s; }

    bool operator==(const Pointer &) const;
};


class Allocator
{
    void *base_ptr;
    size_t size;
    std::vector<bool> is_occupied;
    std::vector<Pointer *> ptrs;

    int getIdxOfPtr(const Pointer &);

public:
    Allocator(void *, size_t);

    Pointer alloc(size_t);
    void realloc(Pointer &, size_t);
    void free(Pointer &);

    void defrag();

    std::string dump();

    ~Allocator();
};

