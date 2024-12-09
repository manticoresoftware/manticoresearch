corrosion_import_crate(
    MANIFEST_PATH Cargo.toml NO_STD
    PROFILE production
)

target_include_directories(rust_hello_world INTERFACE include)
add_library(manticore-rs::hello_world ALIAS rust_hello_world)
