#ifndef RUST_HELLO_WORLD_H
#define RUST_HELLO_WORLD_H

#include <cstddef>
#include <vector>
extern "C" {

void hello_from_rust();
void const_char_ptr_to_stdout_by_rust(const char *, std::vector<double> *, std::size_t);

}
#endif
