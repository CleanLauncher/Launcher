#![allow(clippy::not_unsafe_ptr_arg_deref)]

use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::slice;


macro_rules! ffi_null_check {
    ($($ptr:expr),+ $(,)?) => {
        if $($ptr.is_null())||+ {
            return std::ptr::null_mut();
        }
    };
}

macro_rules! ffi_false_check {
    ($($ptr:expr),+ $(,)?) => {
        if $($ptr.is_null())||+ {
            return false;
        }
    };
}

macro_rules! ffi_cstr_to_str {
    ($ptr:expr) => {
        match unsafe { CStr::from_ptr($ptr) }.to_str() {
            Ok(text) => text,
            Err(_) => return std::ptr::null_mut(),
        }
    };
}

macro_rules! ffi_cstr_to_str_false {
    ($ptr:expr) => {
        match unsafe { CStr::from_ptr($ptr) }.to_str() {
            Ok(text) => text,
            Err(_) => return false,
        }
    };
}

macro_rules! ffi_buffer_to_box {
    ($data:expr, $output_length:expr) => {{
        let mut boxed_buffer = $data.into_boxed_slice();
        unsafe {
            *$output_length = boxed_buffer.len();
        }
        let raw_ptr = boxed_buffer.as_mut_ptr();
        Box::leak(boxed_buffer);
        raw_ptr
    }};
}

macro_rules! ffi_cstring_to_raw {
    ($s:expr) => {
        CString::new($s).unwrap_or_default().into_raw()
    };
}

macro_rules! ffi_string_vec_to_raw {
    ($vec:expr, $out_count:expr) => {{
        let count = $vec.len();
        unsafe {
            *$out_count = count;
        }
        let c_string_vec: Vec<*mut c_char> = $vec
            .into_iter()
            .filter_map(|name| CString::new(name).ok().map(|cs| cs.into_raw()))
            .collect();
        let mut boxed_slice = c_string_vec.into_boxed_slice();
        let raw_ptr = boxed_slice.as_mut_ptr();
        Box::leak(boxed_slice);
        raw_ptr
    }};
}


/// GZip decompression. Returns null on error. Caller must free with `launcher_free_buffer`.
#[no_mangle]
pub extern "C" fn gzip_unzip(
    compressed_ptr: *const u8,
    compressed_len: usize,
    output_length: *mut usize,
) -> *mut u8 {
    ffi_null_check!(compressed_ptr, output_length);
    let input_bytes = unsafe { slice::from_raw_parts(compressed_ptr, compressed_len) };
    match gzip::unzip(input_bytes) {
        Ok(decompressed_payload) => ffi_buffer_to_box!(decompressed_payload, output_length),
        Err(_) => std::ptr::null_mut(),
    }
}

/// GZip compression. Returns null on error. Caller must free with `launcher_free_buffer`.
#[no_mangle]
pub extern "C" fn gzip_zip(
    uncompressed_ptr: *const u8,
    uncompressed_len: usize,
    output_length: *mut usize,
) -> *mut u8 {
    ffi_null_check!(uncompressed_ptr, output_length);
    let input_bytes = unsafe { slice::from_raw_parts(uncompressed_ptr, uncompressed_len) };
    match gzip::zip(input_bytes) {
        Ok(compressed_payload) => ffi_buffer_to_box!(compressed_payload, output_length),
        Err(_) => std::ptr::null_mut(),
    }
}

/// Free a buffer returned by the `gzip_*` functions.
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
pub extern "C" fn markdown_to_html(markdown_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(markdown_ptr);
    let markdown_text = ffi_cstr_to_str!(markdown_ptr);
    match markdown::markdown_to_html(markdown_text) {
        Ok(html_output) => ffi_cstring_to_raw!(html_output),
        Err(_) => std::ptr::null_mut(),
    }
}

/// Free a string returned by `markdown_to_html` or similar functions.
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
    let left_str = match unsafe { CStr::from_ptr(left_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return 0,
    };
    let right_str = match unsafe { CStr::from_ptr(right_ptr) }.to_str() {
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
    ffi_cstring_to_raw!(formatted_size)
}


/// Validate JSON data. Returns `true` when the payload parses.
#[no_mangle]
pub extern "C" fn json_validate(data_ptr: *const u8, data_len: usize) -> bool {
    if data_ptr.is_null() {
        return false;
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    json::parse(input_bytes).is_ok()
}

/// Check whether the data uses the binary JSON format marker.
#[no_mangle]
pub extern "C" fn json_is_binary(data_ptr: *const u8, data_len: usize) -> bool {
    if data_ptr.is_null() {
        return false;
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    json::is_binary_json(input_bytes)
}


/// Read file contents. Returns null on error. Caller must free with `launcher_free_buffer`.
#[no_mangle]
pub extern "C" fn launcher_fs_read(path_ptr: *const c_char, output_length: *mut usize) -> *mut u8 {
    ffi_null_check!(path_ptr, output_length);
    let path_text = ffi_cstr_to_str!(path_ptr);
    match filesystem::read(path_text) {
        Ok(file_payload) => ffi_buffer_to_box!(file_payload, output_length),
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
    ffi_false_check!(path_ptr, data_ptr);
    let path_text = ffi_cstr_to_str_false!(path_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    filesystem::write(path_text, input_bytes).is_ok()
}

/// Delete a path (file or directory tree). Returns `true` on success.
#[no_mangle]
pub extern "C" fn launcher_fs_delete_path(path_ptr: *const c_char) -> bool {
    ffi_false_check!(path_ptr);
    let path_text = ffi_cstr_to_str_false!(path_ptr);
    filesystem::delete_path(path_text).is_ok()
}

/// Strip invalid filename characters. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_fs_remove_invalid_filename_chars(
    input_ptr: *const c_char,
    replace_with: c_char,
) -> *mut c_char {
    ffi_null_check!(input_ptr);
    let input_text = ffi_cstr_to_str!(input_ptr);
    let sanitized =
        filesystem::remove_invalid_filename_chars(input_text, replace_with as u8 as char);
    ffi_cstring_to_raw!(sanitized)
}


/// Compute SHA-256 hash of input bytes. Returns hex string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_hash_sha256(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    ffi_null_check!(data_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    ffi_cstring_to_raw!(hashing::sha256(input_bytes))
}

/// Compute SHA-512 hash of input bytes. Returns hex string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_hash_sha512(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    ffi_null_check!(data_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    ffi_cstring_to_raw!(hashing::sha512(input_bytes))
}

/// Compute MD5 hash of input bytes. Returns hex string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_hash_md5(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    ffi_null_check!(data_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    ffi_cstring_to_raw!(hashing::md5(input_bytes))
}

/// Compute SHA-256 hash of a file. Returns hex string. Caller must free with `launcher_free_string`.
#[no_mangle]
pub extern "C" fn launcher_hash_sha256_file(path_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(path_ptr);
    let path_text = ffi_cstr_to_str!(path_ptr);
    match hashing::sha256_file(path_text) {
        Ok(hash) => ffi_cstring_to_raw!(hash),
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
    ffi_false_check!(data_ptr, expected_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    let expected = ffi_cstr_to_str_false!(expected_ptr);
    hashing::verify_sha256(input_bytes, expected)
}


/// List all entry names in a ZIP archive. Returns null on error. Caller must free with launcher_free_string_list.
#[no_mangle]
pub extern "C" fn launcher_zip_entry_names(
    archive_path_ptr: *const c_char,
    out_count: *mut usize,
) -> *mut *mut c_char {
    ffi_null_check!(archive_path_ptr, out_count);
    let archive_path = ffi_cstr_to_str!(archive_path_ptr);
    match archive::zip_entry_names(archive_path) {
        Ok(names) => ffi_string_vec_to_raw!(names, out_count),
        Err(_) => std::ptr::null_mut(),
    }
}

/// Free a string list returned by launcher_zip_entry_names.
#[no_mangle]
pub extern "C" fn launcher_free_string_list(ptr: *mut *mut c_char, count: usize) {
    if ptr.is_null() {
        return;
    }
    unsafe {
        let slice = std::slice::from_raw_parts_mut(ptr, count);
        for item in slice.iter_mut() {
            if !item.is_null() {
                let _ = CString::from_raw(*item);
            }
        }
        let _ = Box::from_raw(std::ptr::slice_from_raw_parts_mut(ptr, count));
    }
}

/// Read a single entry from a ZIP archive into a buffer. Returns null on error. Caller must free with launcher_free_buffer.
#[no_mangle]
pub extern "C" fn launcher_zip_read_entry(
    archive_path_ptr: *const c_char,
    entry_name_ptr: *const c_char,
    out_len: *mut usize,
) -> *mut u8 {
    ffi_null_check!(archive_path_ptr, entry_name_ptr, out_len);
    let archive_path = ffi_cstr_to_str!(archive_path_ptr);
    let entry_name = ffi_cstr_to_str!(entry_name_ptr);
    match archive::zip_read_entry(archive_path, entry_name) {
        Ok(entry_data) => ffi_buffer_to_box!(entry_data, out_len),
        Err(_) => std::ptr::null_mut(),
    }
}

/// Extract a single file from a ZIP archive to disk. Returns true on success.
#[no_mangle]
pub extern "C" fn launcher_zip_extract_file(
    archive_path_ptr: *const c_char,
    entry_name_ptr: *const c_char,
    target_path_ptr: *const c_char,
) -> bool {
    ffi_false_check!(archive_path_ptr, entry_name_ptr, target_path_ptr);
    let archive_path = ffi_cstr_to_str_false!(archive_path_ptr);
    let entry_name = ffi_cstr_to_str_false!(entry_name_ptr);
    let target_path = ffi_cstr_to_str_false!(target_path_ptr);
    archive::zip_extract_file(archive_path, entry_name, target_path).is_ok()
}

/// Extract a directory from a ZIP archive to disk. Returns extracted file count, or -1 on error.
/// Caller must free result names with launcher_free_string_list.
#[no_mangle]
pub extern "C" fn launcher_zip_extract_dir(
    archive_path_ptr: *const c_char,
    subdir_prefix_ptr: *const c_char,
    target_dir_ptr: *const c_char,
    out_count: *mut usize,
) -> *mut *mut c_char {
    ffi_null_check!(archive_path_ptr, target_dir_ptr, out_count);
    let archive_path = ffi_cstr_to_str!(archive_path_ptr);
    let subdir_prefix = if subdir_prefix_ptr.is_null() {
        ""
    } else {
        ffi_cstr_to_str!(subdir_prefix_ptr)
    };
    let target_dir = ffi_cstr_to_str!(target_dir_ptr);
    match archive::zip_extract_dir(archive_path, subdir_prefix, target_dir) {
        Ok(files) => ffi_string_vec_to_raw!(files, out_count),
        Err(_) => std::ptr::null_mut(),
    }
}

/// Check if an entry exists in a ZIP archive. Returns true if found.
#[no_mangle]
pub extern "C" fn launcher_zip_entry_exists(
    archive_path_ptr: *const c_char,
    entry_name_ptr: *const c_char,
) -> bool {
    ffi_false_check!(archive_path_ptr, entry_name_ptr);
    let archive_path = ffi_cstr_to_str_false!(archive_path_ptr);
    let entry_name = ffi_cstr_to_str_false!(entry_name_ptr);
    archive::zip_entry_exists(archive_path, entry_name).unwrap_or(false)
}


/// List entries in a TAR.GZ archive. Returns null on error. Caller must free with launcher_free_string_list.
#[no_mangle]
pub extern "C" fn launcher_tar_entry_names(
    archive_path_ptr: *const c_char,
    out_count: *mut usize,
) -> *mut *mut c_char {
    ffi_null_check!(archive_path_ptr, out_count);
    let archive_path = ffi_cstr_to_str!(archive_path_ptr);
    match archive::tar_list_entries(archive_path) {
        Ok(entries) => {
            let name_list: Vec<String> = entries.into_iter().map(|e| e.entry_name).collect();
            ffi_string_vec_to_raw!(name_list, out_count)
        }
        Err(_) => std::ptr::null_mut(),
    }
}

/// Extract a TAR.GZ archive to disk. Returns extracted file count, or -1 on error.
/// Caller must free result names with launcher_free_string_list.
#[no_mangle]
pub extern "C" fn launcher_tar_extract_dir(
    archive_path_ptr: *const c_char,
    target_dir_ptr: *const c_char,
    out_count: *mut usize,
) -> *mut *mut c_char {
    ffi_null_check!(archive_path_ptr, target_dir_ptr, out_count);
    let archive_path = ffi_cstr_to_str!(archive_path_ptr);
    let target_dir = ffi_cstr_to_str!(target_dir_ptr);
    match archive::tar_extract_dir(archive_path, target_dir) {
        Ok(files) => ffi_string_vec_to_raw!(files, out_count),
        Err(_) => std::ptr::null_mut(),
    }
}
