# Rust usage in ManticoreSearch

This directory contains all rust modules, used by ManticoreSearch.
The Simple "hello world" project is a toy example for rust integration into C++, and will be replaced with 
something more representive and useful. C++ usage of this example may be like:
```cpp
include "rust_hello_world.h"

int main ( int argc, char ** argv )
{
    hello_from_rust();
}
```
# I want to add some Rust code, what do I need to do?

- ```shell
  # manticoresearch/rust directory
  cargo new lib_name --lib # replace "lib_name" with your name  
  ```
- Edit your Cargo.toml 
  - ```toml
    [lib]
    crate-type = ["staticlib"]
    ```
    Set crate type to "staticlib"
  - ```toml
    [profile.production]
    inherits = "release"
    lto = true
    codegen-units = 1    
    ````
    LTO and Codegen Units are options for Rust compiler, usually provides better performance. You may experiment with 
    them.
    Production profile will be linked with ManticoreSearch C++ code.
- Place `CMakeLists.txt` file with followed content 
  ([additional options](https://corrosion-rs.github.io/corrosion/usage.html)):
  ```
  corrosion_import_crate(
      MANIFEST_PATH Cargo.toml
      PROFILE production
  )

  target_include_directories(_rust_hello_world INTERFACE include)
  add_library(manticore-rs::hello_world ALIAS _rust_hello_world)
  ```
  Change "rust_hello_world" and "hello_world" to yours. 
  Notice that "rust_hello_world" should be linked to 
  ```
  [package]
  name = "rust_hello_world" 
  ``` 
- Make directory named "include" and place header file inside. Refer to rust_hello_world project 
  example.
- Finally, edit `CMakeLists.txt` file in "rust" directory
  ```
  add_subdirectory( %your_project_name% )
  ```
  with name of your Rust project.
- Edit `src/CMakeLists.txt` to add your Rust code into binary
  ```text
  # RUST SECTION
  target_link_libraries ( searchd manticore-rs::hello_world )
  # add here your project
  ```
# Some code agreement

- Notice that "rust_hello_world" project doesn't have tests, because of extreme simplicity. Don't ignore tests.
- __Don't allow your production code to panic!__
  No ```unwrap()``` like here:
    ```rust
    let example = std::fs::read_dir("/path/to/file").unwrap();
    ```
  It's ok to your code panic while prototyping, etc., but not in production mode.
- Use cargo clippy to format your code properly.
  You may try to use clippy flags, like here:
  ```shell
  cargo clippy \
  -W clippy::pedantic \
  -W clippy::nursery \
  -W clippy::unwrap_used 
  ```
- Some useful tools. Mark them as dev-dependency in Cargo.toml.
  - [Doctests](https://doc.rust-lang.org/rustdoc/how-to-write-documentation.html)
  - Property testing with [proptest](https://github.com/proptest-rs/proptest)
  - Fuzz testing with [cargo-fuzz](https://rust-fuzz.github.io/book/cargo-fuzz.html)
  - Mutation testing with [mutagen](https://github.com/llogiq/mutagen), but be careful with that one
  - Microbenchmarks. Look at [criterion-rs](https://github.com/bheisler/criterion.rs) 
    and [iai](https://github.com/bheisler/iai)
- I may forget to mention something. Welcome to https://github.com/manticoresoftware/manticoresearch/issues 
