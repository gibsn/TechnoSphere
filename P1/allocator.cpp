#include "allocator.h"


Pointer::Pointer()
    : ptr(new void *(nullptr)),
    size(new size_t(0))
{}


Pointer::Pointer(void **p, size_t s)
    : ptr(p),
    size(new size_t(s))
{}


bool Pointer::operator==(const Pointer &p) const
{
    if (ptr == p.ptr && size == p.size)
        return true;
    else
        return false;
}


Allocator::Allocator(void *base, size_t s)
    : base_ptr(base),
    size(s),
    is_occupied(std::vector<bool>(s, 0)),
    ptrs(std::vector<Pointer *>())
{
}


int Allocator::getIdxOfPtr(const Pointer &p)
{
    for (int idx_of_pointer = 0; idx_of_pointer < ptrs.size(); ++idx_of_pointer)
        if (*(ptrs[idx_of_pointer]) == p)
            return idx_of_pointer;

    return -1;
}


Pointer Allocator::alloc(size_t N)
{
    size_t bytes_counter = 0;
    int idx_of_start = 0;
    bool enough_memory = false;
    for (int i = 0; i < size; ++i)
    {
        if (!is_occupied[i])
            ++bytes_counter;
        else
        {
            idx_of_start = i + 1;
            bytes_counter = 0;
        }

        if (bytes_counter == N)
        {
            enough_memory = true;
            break;
        }
    }
    if (!enough_memory)
        throw AllocError(AllocErrorType::NoMemory, "Not enough memory\n");

    for (int i = 0; i < N; ++i)
        is_occupied[idx_of_start + i] = true;
    void **new_ptr = new void *((char *)base_ptr + idx_of_start);
    ptrs.push_back(new Pointer(new_ptr, N));

    return *ptrs[ptrs.size() - 1];
}


void Allocator::realloc(Pointer &p, size_t N)
{
    if (p.getSize() == 0)
    {
        p = alloc(N);
        return;
    }

    int offset = (char *)p.get() - (char *)base_ptr;
    if (N < p.getSize())
    {
        for (int i = N; i < p.getSize(); ++i)
            is_occupied[offset + i] = false;
        p.setSize(N);
        return;
    }

    //Check if we can extend it in place
    bool extendable = true;
    for (int i = p.getSize(); i < N; ++i)
        if (is_occupied[offset + i])
        {
            extendable = false;
            break;
        }

    if (extendable)
    {
        for (int i = p.getSize(); i < N; ++i)
            is_occupied[offset + i] = true;
        p.setSize(N);
    }
    else
    {
        Pointer new_ptr = alloc(N);
        memcpy(new_ptr.get(), p.get(), p.getSize());
        Allocator::free(p);
        p = new_ptr;
    }
}


void Allocator::free(Pointer &p)
{
    int offset = (char *)p.get() - (char *)base_ptr;
    int idx_of_pointer;
    if ((idx_of_pointer = getIdxOfPtr(p)) == -1)
        throw AllocError(AllocErrorType::InvalidFree, "Invalid free\n");

    for (int i = 0; i < p.getSize(); ++i)
        is_occupied[offset + i] = false;
    delete ptrs[idx_of_pointer];
    ptrs.erase(ptrs.begin() + idx_of_pointer);
    p = Pointer(new void *(nullptr), 0);
}


void Allocator::defrag()
{
    sort(ptrs.begin(), ptrs.end(), [](Pointer *p1, Pointer *p2) -> bool
    {
        return p1->get() < p1->get();
    });

    char *dst = (char *)base_ptr;
    int whole_length = 0;
    for (int i = 0; i < ptrs.size(); ++i)
    {
        int curr_size = ptrs[i]->getSize();
        memmove(dst, ptrs[i]->get(), curr_size);
        ptrs[i]->setPtr(dst);
        whole_length += curr_size;
        dst += curr_size;
    }
    for (int i = 0; i < whole_length; ++i)
        is_occupied[i] = true;
    for (int i = whole_length; i < size; ++i)
        is_occupied[i] = false;
}


std::string Allocator::dump()
{
    return "";
}


Allocator::~Allocator()
{
    for (int i = 0; i < ptrs.size(); ++i)
        delete ptrs[i];
}