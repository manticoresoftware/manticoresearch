#[no_mangle]
pub extern "C" fn hello_from_rust() {
    println!("Good morning from Rust programming language!");
}

#[cfg(test)]
mod tests {}
