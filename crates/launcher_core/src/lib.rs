#![allow(clippy::not_unsafe_ptr_arg_deref)]

use launcher_archive as archive;
use launcher_filesystem as filesystem;
use launcher_gzip as gzip;
use launcher_hashing as hashing;
use launcher_json as json;
use launcher_markdown as markdown;
use launcher_string_utils as string_utils;

use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::slice;

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

/// List all entry names in a ZIP archive. Returns null on error. Caller must free with launcher_free_string_list.
#[no_mangle]
pub extern "C" fn launcher_zip_entry_names(
    archive_path_ptr: *const c_char,
    out_count: *mut usize,
) -> *mut *mut c_char {
    if archive_path_ptr.is_null() || out_count.is_null() {
        return std::ptr::null_mut();
    }
    let archive_path = match unsafe { CStr::from_ptr(archive_path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    let entry_names = match archive::zip_entry_names(archive_path) {
        Ok(names) => names,
        Err(_) => return std::ptr::null_mut(),
    };
    let count = entry_names.len();
    unsafe {
        *out_count = count;
    }
    let c_string_vec: Vec<*mut c_char> = entry_names
        .into_iter()
        .filter_map(|name| CString::new(name).ok().map(|cs| cs.into_raw()))
        .collect();
    let mut boxed_slice = c_string_vec.into_boxed_slice();
    let raw_ptr = boxed_slice.as_mut_ptr();
    Box::leak(boxed_slice);
    raw_ptr
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
    if archive_path_ptr.is_null() || entry_name_ptr.is_null() || out_len.is_null() {
        return std::ptr::null_mut();
    }
    let archive_path = match unsafe { CStr::from_ptr(archive_path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    let entry_name = match unsafe { CStr::from_ptr(entry_name_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    match archive::zip_read_entry(archive_path, entry_name) {
        Ok(entry_data) => {
            unsafe {
                *out_len = entry_data.len();
            }
            let mut boxed_buffer = entry_data.into_boxed_slice();
            let raw_ptr = boxed_buffer.as_mut_ptr();
            Box::leak(boxed_buffer);
            raw_ptr
        }
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
    if archive_path_ptr.is_null() || entry_name_ptr.is_null() || target_path_ptr.is_null() {
        return false;
    }
    let archive_path = match unsafe { CStr::from_ptr(archive_path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return false,
    };
    let entry_name = match unsafe { CStr::from_ptr(entry_name_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return false,
    };
    let target_path = match unsafe { CStr::from_ptr(target_path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return false,
    };
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
    if archive_path_ptr.is_null() || target_dir_ptr.is_null() || out_count.is_null() {
        return std::ptr::null_mut();
    }
    let archive_path = match unsafe { CStr::from_ptr(archive_path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    let subdir_prefix = if subdir_prefix_ptr.is_null() {
        ""
    } else {
        match unsafe { CStr::from_ptr(subdir_prefix_ptr) }.to_str() {
            Ok(text) => text,
            Err(_) => return std::ptr::null_mut(),
        }
    };
    let target_dir = match unsafe { CStr::from_ptr(target_dir_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    let extracted = match archive::zip_extract_dir(archive_path, subdir_prefix, target_dir) {
        Ok(files) => files,
        Err(_) => return std::ptr::null_mut(),
    };
    let count = extracted.len();
    unsafe {
        *out_count = count;
    }
    let c_string_vec: Vec<*mut c_char> = extracted
        .into_iter()
        .filter_map(|name| CString::new(name).ok().map(|cs| cs.into_raw()))
        .collect();
    let mut boxed_slice = c_string_vec.into_boxed_slice();
    let raw_ptr = boxed_slice.as_mut_ptr();
    Box::leak(boxed_slice);
    raw_ptr
}

/// Check if an entry exists in a ZIP archive. Returns true if found.
#[no_mangle]
pub extern "C" fn launcher_zip_entry_exists(
    archive_path_ptr: *const c_char,
    entry_name_ptr: *const c_char,
) -> bool {
    if archive_path_ptr.is_null() || entry_name_ptr.is_null() {
        return false;
    }
    let archive_path = match unsafe { CStr::from_ptr(archive_path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return false,
    };
    let entry_name = match unsafe { CStr::from_ptr(entry_name_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return false,
    };
    archive::zip_entry_exists(archive_path, entry_name).unwrap_or(false)
}

/// List entries in a TAR.GZ archive. Returns null on error. Caller must free with launcher_free_string_list.
#[no_mangle]
pub extern "C" fn launcher_tar_entry_names(
    archive_path_ptr: *const c_char,
    out_count: *mut usize,
) -> *mut *mut c_char {
    if archive_path_ptr.is_null() || out_count.is_null() {
        return std::ptr::null_mut();
    }
    let archive_path = match unsafe { CStr::from_ptr(archive_path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    let entries = match archive::tar_list_entries(archive_path) {
        Ok(entry_list) => entry_list,
        Err(_) => return std::ptr::null_mut(),
    };
    let name_list: Vec<String> = entries.into_iter().map(|e| e.entry_name).collect();
    let count = name_list.len();
    unsafe {
        *out_count = count;
    }
    let c_string_vec: Vec<*mut c_char> = name_list
        .into_iter()
        .filter_map(|name| CString::new(name).ok().map(|cs| cs.into_raw()))
        .collect();
    let mut boxed_slice = c_string_vec.into_boxed_slice();
    let raw_ptr = boxed_slice.as_mut_ptr();
    Box::leak(boxed_slice);
    raw_ptr
}

/// Extract a TAR.GZ archive to disk. Returns extracted file count, or -1 on error.
/// Caller must free result names with launcher_free_string_list.
#[no_mangle]
pub extern "C" fn launcher_tar_extract_dir(
    archive_path_ptr: *const c_char,
    target_dir_ptr: *const c_char,
    out_count: *mut usize,
) -> *mut *mut c_char {
    if archive_path_ptr.is_null() || target_dir_ptr.is_null() || out_count.is_null() {
        return std::ptr::null_mut();
    }
    let archive_path = match unsafe { CStr::from_ptr(archive_path_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    let target_dir = match unsafe { CStr::from_ptr(target_dir_ptr) }.to_str() {
        Ok(text) => text,
        Err(_) => return std::ptr::null_mut(),
    };
    let extracted = match archive::tar_extract_dir(archive_path, target_dir) {
        Ok(files) => files,
        Err(_) => return std::ptr::null_mut(),
    };
    let count = extracted.len();
    unsafe {
        *out_count = count;
    }
    let c_string_vec: Vec<*mut c_char> = extracted
        .into_iter()
        .filter_map(|name| CString::new(name).ok().map(|cs| cs.into_raw()))
        .collect();
    let mut boxed_slice = c_string_vec.into_boxed_slice();
    let raw_ptr = boxed_slice.as_mut_ptr();
    Box::leak(boxed_slice);
    raw_ptr
}
