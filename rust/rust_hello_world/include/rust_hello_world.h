#ifndef RUST_HELLO_WORLD_H
#define RUST_HELLO_WORLD_H

extern "C" {
void hello_from_rust();
void const_char_ptr_to_stdout_by_rust(const char *);
}
#endif
