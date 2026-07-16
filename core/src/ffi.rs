#![allow(clippy::not_unsafe_ptr_arg_deref)]

use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::slice;

use crate::{filesystem, gzip, hashing, json, markdown, string_utils};

/// GZip decompression. Returns null on error. Caller must free with `launcher_free_buffer`.
#[no_mangle]
pub extern "C" fn launcher_gzip_unzip(
    compressed_ptr: *const u8,
    compressed_len: usize,
    output_length: *mut usize,
) -> *mut u8 {
    if compressed_ptr.is_null() || output_length.is_null() {
        return std::ptr::null_mut();
    }
    let input_bytes = unsafe { slice::from_raw_parts(compressed_ptr, compressed_len) };
    match gzip::unzip(input_bytes) {
        Ok(decompressed_payload) => {
            unsafe {
                *output_length = decompressed_payload.len();
            }
            let mut boxed_buffer = decompressed_payload.into_boxed_slice();
            let raw_ptr = boxed_buffer.as_mut_ptr();
            Box::leak(boxed_buffer);
            raw_ptr
        }
        Err(_) => std::ptr::null_mut(),
    }
}

/// GZip compression. Returns null on error. Caller must free with `launcher_free_buffer`.
#[no_mangle]
pub extern "C" fn launcher_gzip_zip(
    uncompressed_ptr: *const u8,
    uncompressed_len: usize,
    output_length: *mut usize,
) -> *mut u8 {
    if uncompressed_ptr.is_null() || output_length.is_null() {
        return std::ptr::null_mut();
    }
    let input_bytes = unsafe { slice::from_raw_parts(uncompressed_ptr, uncompressed_len) };
    match gzip::zip(input_bytes) {
        Ok(compressed_payload) => {
            unsafe {
                *output_length = compressed_payload.len();
            }
            let mut boxed_buffer = compressed_payload.into_boxed_slice();
            let raw_ptr = boxed_buffer.as_mut_ptr();
            Box::leak(boxed_buffer);
            raw_ptr
        }
        Err(_) => std::ptr::null_mut(),
    }
}

/// Free a buffer returned by the `launcher_gzip_*` functions.
#[no_mangle]
pub extern "C" fn launcher_free_buffer(raw_ptr: *mut u8, length: usize) {
    if !raw_ptr.is_null() {
        unsafe {
            let _ = Box::from_raw(std::ptr::slice_from_raw_parts_mut(raw_ptr, length));
        }
    }
}

/// Convert markdown to HTML. Returns null on error. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_markdown_to_html(markdown_ptr: *const c_char) -> *mut c_char {
    if markdown_ptr.is_null() {
        return std::ptr::null_mut();
    }
    let markdown_cstr = unsafe { CStr::from_ptr(markdown_ptr) };
    let markdown_text = match markdown_cstr.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    match markdown::markdown_to_html(markdown_text) {
        Ok(html_output) => CString::new(html_output).unwrap_or_default().into_raw(),
        Err(_) => std::ptr::null_mut(),
    }
}

/// Free a string returned by `launcher_markdown_to_html` or similar functions.
#[no_mangle]
pub extern "C" fn launcher_free_string(raw_ptr: *mut c_char) {
    if !raw_ptr.is_null() {
        unsafe {
            let _ = CString::from_raw(raw_ptr);
        }
    }
}

/// Natural compare of two strings. Returns `-1`, `0`, or `1`.
#[no_mangle]
pub extern "C" fn launcher_natural_compare(
    left_ptr: *const c_char,
    right_ptr: *const c_char,
    case_insensitive: bool,
) -> i32 {
    if left_ptr.is_null() || right_ptr.is_null() {
        return 0;
    }
    let left_cstr = unsafe { CStr::from_ptr(left_ptr) };
    let right_cstr = unsafe { CStr::from_ptr(right_ptr) };
    let left_str = match left_cstr.to_str() {
        Ok(text) => text,
        Err(_) => return 0,
    };
    let right_str = match right_cstr.to_str() {
        Ok(text) => text,
        Err(_) => return 0,
    };
    match string_utils::natural_compare(left_str, right_str, case_insensitive) {
        std::cmp::Ordering::Less => -1,
        std::cmp::Ordering::Equal => 0,
        std::cmp::Ordering::Greater => 1,
    }
}

/// Format a file size as a human readable string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_human_readable_file_size(
    raw_byte_count: f64,
    use_si_units: bool,
    decimal_points: usize,
) -> *mut c_char {
    let formatted_size =
        string_utils::human_readable_file_size(raw_byte_count, use_si_units, decimal_points);
    CString::new(formatted_size).unwrap_or_default().into_raw()
}

/// Validate JSON data. Returns `true` when the payload parses.
#[no_mangle]
pub extern "C" fn launcher_json_validate(data_ptr: *const u8, data_len: usize) -> bool {
    if data_ptr.is_null() {
        return false;
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    json::parse(input_bytes).is_ok()
}

/// Check whether the data uses the binary JSON format marker.
#[no_mangle]
pub extern "C" fn launcher_json_is_binary(data_ptr: *const u8, data_len: usize) -> bool {
    if data_ptr.is_null() {
        return false;
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    json::is_binary_json(input_bytes)
}

/// Read file contents. Returns null on error. Caller must free with `launcher_free_buffer`.
#[no_mangle]
pub extern "C" fn launcher_fs_read(path_ptr: *const c_char, output_length: *mut usize) -> *mut u8 {
    if path_ptr.is_null() || output_length.is_null() {
        return std::ptr::null_mut();
    }
    let path_cstr = unsafe { CStr::from_ptr(path_ptr) };
    let path_text = match path_cstr.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    match filesystem::read(path_text) {
        Ok(file_payload) => {
            unsafe {
                *output_length = file_payload.len();
            }
            let mut boxed_buffer = file_payload.into_boxed_slice();
            let raw_ptr = boxed_buffer.as_mut_ptr();
            Box::leak(boxed_buffer);
            raw_ptr
        }
        Err(_) => std::ptr::null_mut(),
    }
}

/// Write data to a file. Returns `true` on success.
#[no_mangle]
pub extern "C" fn launcher_fs_write(
    path_ptr: *const c_char,
    data_ptr: *const u8,
    data_len: usize,
) -> bool {
    if path_ptr.is_null() || data_ptr.is_null() {
        return false;
    }
    let path_cstr = unsafe { CStr::from_ptr(path_ptr) };
    let path_text = match path_cstr.to_str() {
        Ok(text) => text,
        Err(_) => return false,
    };
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    filesystem::write(path_text, input_bytes).is_ok()
}

/// Delete a path (file or directory tree). Returns `true` on success.
#[no_mangle]
pub extern "C" fn launcher_fs_delete_path(path_ptr: *const c_char) -> bool {
    if path_ptr.is_null() {
        return false;
    }
    let path_cstr = unsafe { CStr::from_ptr(path_ptr) };
    let path_text = match path_cstr.to_str() {
        Ok(text) => text,
        Err(_) => return false,
    };
    filesystem::delete_path(path_text).is_ok()
}

/// Strip invalid filename characters. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_fs_remove_invalid_filename_chars(
    input_ptr: *const c_char,
    replace_with: c_char,
) -> *mut c_char {
    if input_ptr.is_null() {
        return std::ptr::null_mut();
    }
    let input_cstr = unsafe { CStr::from_ptr(input_ptr) };
    let input_text = match input_cstr.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    let sanitized =
        filesystem::remove_invalid_filename_chars(input_text, replace_with as u8 as char);
    CString::new(sanitized).unwrap_or_default().into_raw()
}

/// Compute SHA-256 hash of input bytes. Returns hex string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_hash_sha256(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    if data_ptr.is_null() {
        return std::ptr::null_mut();
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    CString::new(hashing::sha256(input_bytes))
        .unwrap_or_default()
        .into_raw()
}

/// Compute SHA-512 hash of input bytes. Returns hex string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_hash_sha512(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    if data_ptr.is_null() {
        return std::ptr::null_mut();
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    CString::new(hashing::sha512(input_bytes))
        .unwrap_or_default()
        .into_raw()
}

/// Compute MD5 hash of input bytes. Returns hex string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_hash_md5(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    if data_ptr.is_null() {
        return std::ptr::null_mut();
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    CString::new(hashing::md5(input_bytes))
        .unwrap_or_default()
        .into_raw()
}

/// Compute SHA-256 hash of a file. Returns hex string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_hash_sha256_file(path_ptr: *const c_char) -> *mut c_char {
    if path_ptr.is_null() {
        return std::ptr::null_mut();
    }
    let path_text = match unsafe { CStr::from_ptr(path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    match hashing::sha256_file(path_text) {
        Ok(hash) => CString::new(hash).unwrap_or_default().into_raw(),
        Err(_) => std::ptr::null_mut(),
    }
}

/// Verify SHA-256 hash of input bytes against expected hash. Returns true on match.
#[no_mangle]
pub extern "C" fn launcher_verify_sha256(
    data_ptr: *const u8,
    data_len: usize,
    expected_ptr: *const c_char,
) -> bool {
    if data_ptr.is_null() || expected_ptr.is_null() {
        return false;
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    let expected = match unsafe { CStr::from_ptr(expected_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return false,
    };
    hashing::verify_sha256(input_bytes, expected)
}
