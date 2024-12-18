use core::slice;
use std::ffi::CStr;
use std::os::raw::c_char;
use std::ptr::null;

#[no_mangle]
pub extern "C" fn hello_from_rust() {
    println!("Good morning from Rust programming language!");
}

#[no_mangle]
pub unsafe extern "C" fn const_char_ptr_to_stdout_by_rust(
    c_str: *const c_char,
    data: *mut f64,
    len: usize,
) {
    let sl = slice::from_raw_parts_mut(data, len);
    if c_str.is_null() {
        return;
    }
    unsafe {
        let s = CStr::from_ptr(c_str);
        println!("From Rust: {:#?}", s);
    }
    sl.iter_mut()
        .enumerate()
        .for_each(|(index, x)| *x = index as f64);
    println!("From Rust slice: {:#?}", sl);
}

#[cfg(test)]
mod tests {}
