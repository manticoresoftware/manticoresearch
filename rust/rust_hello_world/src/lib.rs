use std::ffi::CStr;
use std::os::raw::c_char;

#[no_mangle]
pub extern "C" fn hello_from_rust() {
    println!("Good morning from Rust programming language!");
}

#[no_mangle]
pub unsafe extern "C" fn const_char_ptr_to_stdout_by_rust(c_str: *const c_char) {
    if c_str.is_null() {
        return;
    }
    unsafe {
        let s = CStr::from_ptr(c_str);
        println!("From Rust: {:#?}", s);
    }
}

#[cfg(test)]
mod tests {}
