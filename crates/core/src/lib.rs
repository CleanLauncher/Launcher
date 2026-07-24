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

macro_rules! ffi_cstr_to_str_void {
    ($ptr:expr) => {
        match unsafe { CStr::from_ptr($ptr) }.to_str() {
            Ok(text) => text,
            Err(_) => return,
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

#[no_mangle]
pub extern "C" fn gzip_unzip(compressed_ptr: *const u8, compressed_len: usize, output_length: *mut usize) -> *mut u8 {
    ffi_null_check!(compressed_ptr, output_length);
    let input_bytes = unsafe { slice::from_raw_parts(compressed_ptr, compressed_len) };
    match gzip::unzip(input_bytes) {
        Ok(decompressed_payload) => ffi_buffer_to_box!(decompressed_payload, output_length),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn gzip_zip(uncompressed_ptr: *const u8, uncompressed_len: usize, output_length: *mut usize) -> *mut u8 {
    ffi_null_check!(uncompressed_ptr, output_length);
    let input_bytes = unsafe { slice::from_raw_parts(uncompressed_ptr, uncompressed_len) };
    match gzip::zip(input_bytes) {
        Ok(compressed_payload) => ffi_buffer_to_box!(compressed_payload, output_length),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_free_buffer(raw_ptr: *mut u8, length: usize) {
    if !raw_ptr.is_null() {
        unsafe {
            let _ = Box::from_raw(std::ptr::slice_from_raw_parts_mut(raw_ptr, length));
        }
    }
}

#[no_mangle]
pub extern "C" fn markdown_to_html(markdown_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(markdown_ptr);
    let markdown_text = ffi_cstr_to_str!(markdown_ptr);
    match markdown::markdown_to_html(markdown_text) {
        Ok(html_output) => ffi_cstring_to_raw!(html_output),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_free_string(raw_ptr: *mut c_char) {
    if !raw_ptr.is_null() {
        unsafe {
            let _ = CString::from_raw(raw_ptr);
        }
    }
}

#[no_mangle]
pub extern "C" fn launcher_natural_compare(left_ptr: *const c_char, right_ptr: *const c_char, case_insensitive: bool) -> i32 {
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

#[no_mangle]
pub extern "C" fn launcher_human_readable_file_size(raw_byte_count: f64, use_si_units: bool, decimal_points: usize) -> *mut c_char {
    let formatted_size = string_utils::human_readable_file_size(raw_byte_count, use_si_units, decimal_points);
    ffi_cstring_to_raw!(formatted_size)
}

#[no_mangle]
pub extern "C" fn json_validate(data_ptr: *const u8, data_len: usize) -> bool {
    if data_ptr.is_null() {
        return false;
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    json::parse(input_bytes).is_ok()
}

#[no_mangle]
pub extern "C" fn json_is_binary(data_ptr: *const u8, data_len: usize) -> bool {
    if data_ptr.is_null() {
        return false;
    }
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    json::is_binary_json(input_bytes)
}

#[no_mangle]
pub extern "C" fn launcher_fs_read(path_ptr: *const c_char, output_length: *mut usize) -> *mut u8 {
    ffi_null_check!(path_ptr, output_length);
    let path_text = ffi_cstr_to_str!(path_ptr);
    match filesystem::read(path_text) {
        Ok(file_payload) => ffi_buffer_to_box!(file_payload, output_length),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_fs_write(path_ptr: *const c_char, data_ptr: *const u8, data_len: usize) -> bool {
    ffi_false_check!(path_ptr, data_ptr);
    let path_text = ffi_cstr_to_str_false!(path_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    filesystem::write(path_text, input_bytes).is_ok()
}

#[no_mangle]
pub extern "C" fn launcher_fs_delete_path(path_ptr: *const c_char) -> bool {
    ffi_false_check!(path_ptr);
    let path_text = ffi_cstr_to_str_false!(path_ptr);
    filesystem::delete_path(path_text).is_ok()
}

#[no_mangle]
pub extern "C" fn launcher_fs_remove_invalid_filename_chars(input_ptr: *const c_char, replace_with: c_char) -> *mut c_char {
    ffi_null_check!(input_ptr);
    let input_text = ffi_cstr_to_str!(input_ptr);
    let sanitized = filesystem::remove_invalid_filename_chars(input_text, replace_with as u8 as char);
    ffi_cstring_to_raw!(sanitized)
}

#[no_mangle]
pub extern "C" fn launcher_hash_sha256(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    ffi_null_check!(data_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    ffi_cstring_to_raw!(hashing::sha256(input_bytes))
}

#[no_mangle]
pub extern "C" fn launcher_hash_sha512(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    ffi_null_check!(data_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    ffi_cstring_to_raw!(hashing::sha512(input_bytes))
}

#[no_mangle]
pub extern "C" fn launcher_hash_md5(data_ptr: *const u8, data_len: usize) -> *mut c_char {
    ffi_null_check!(data_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    ffi_cstring_to_raw!(hashing::md5(input_bytes))
}

#[no_mangle]
pub extern "C" fn launcher_hash_sha256_file(path_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(path_ptr);
    let path_text = ffi_cstr_to_str!(path_ptr);
    match hashing::sha256_file(path_text) {
        Ok(hash) => ffi_cstring_to_raw!(hash),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_verify_sha256(data_ptr: *const u8, data_len: usize, expected_ptr: *const c_char) -> bool {
    ffi_false_check!(data_ptr, expected_ptr);
    let input_bytes = unsafe { slice::from_raw_parts(data_ptr, data_len) };
    let expected = ffi_cstr_to_str_false!(expected_ptr);
    hashing::verify_sha256(input_bytes, expected)
}

#[no_mangle]
pub extern "C" fn launcher_zip_entry_names(archive_path_ptr: *const c_char, out_count: *mut usize) -> *mut *mut c_char {
    ffi_null_check!(archive_path_ptr, out_count);
    let archive_path = ffi_cstr_to_str!(archive_path_ptr);
    match archive::zip_entry_names(archive_path) {
        Ok(names) => ffi_string_vec_to_raw!(names, out_count),
        Err(_) => std::ptr::null_mut(),
    }
}

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

#[no_mangle]
pub extern "C" fn launcher_zip_read_entry(archive_path_ptr: *const c_char, entry_name_ptr: *const c_char, out_len: *mut usize) -> *mut u8 {
    ffi_null_check!(archive_path_ptr, entry_name_ptr, out_len);
    let archive_path = ffi_cstr_to_str!(archive_path_ptr);
    let entry_name = ffi_cstr_to_str!(entry_name_ptr);
    match archive::zip_read_entry(archive_path, entry_name) {
        Ok(entry_data) => ffi_buffer_to_box!(entry_data, out_len),
        Err(_) => std::ptr::null_mut(),
    }
}

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

#[no_mangle]
pub extern "C" fn launcher_zip_entry_exists(archive_path_ptr: *const c_char, entry_name_ptr: *const c_char) -> bool {
    ffi_false_check!(archive_path_ptr, entry_name_ptr);
    let archive_path = ffi_cstr_to_str_false!(archive_path_ptr);
    let entry_name = ffi_cstr_to_str_false!(entry_name_ptr);
    archive::zip_entry_exists(archive_path, entry_name).unwrap_or(false)
}

#[no_mangle]
pub extern "C" fn launcher_tar_entry_names(archive_path_ptr: *const c_char, out_count: *mut usize) -> *mut *mut c_char {
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

#[no_mangle]
pub extern "C" fn launcher_http_set_user_agent(agent_ptr: *const c_char) {
    if agent_ptr.is_null() {
        return;
    }
    let agent = ffi_cstr_to_str_void!(agent_ptr);
    http_client::set_user_agent(agent);
}

#[no_mangle]
pub extern "C" fn launcher_http_set_timeout(timeout_ms: u64) {
    http_client::set_timeout(timeout_ms);
}

#[no_mangle]
pub extern "C" fn launcher_http_set_header(name_ptr: *const c_char, value_ptr: *const c_char) {
    if name_ptr.is_null() || value_ptr.is_null() {
        return;
    }
    let name = ffi_cstr_to_str_void!(name_ptr);
    let value = ffi_cstr_to_str_void!(value_ptr);
    http_client::set_header(name, value);
}

#[no_mangle]
pub extern "C" fn launcher_http_get(url_ptr: *const c_char, out_status: *mut u16, out_len: *mut usize) -> *mut u8 {
    ffi_null_check!(url_ptr, out_status, out_len);
    let url = ffi_cstr_to_str!(url_ptr);
    match http_client::get(url) {
        Ok(response) => {
            unsafe {
                *out_status = response.status;
            }
            ffi_buffer_to_box!(response.body, out_len)
        }
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_http_get_file(url_ptr: *const c_char, path_ptr: *const c_char, out_status: *mut u16) -> bool {
    ffi_false_check!(url_ptr, path_ptr);
    let url = ffi_cstr_to_str_false!(url_ptr);
    let path = ffi_cstr_to_str_false!(path_ptr);
    match http_client::download_to_file(url, path) {
        Ok(result) => {
            unsafe {
                *out_status = result.status;
            }
            true
        }
        Err(_) => false,
    }
}

#[no_mangle]
pub extern "C" fn launcher_http_get_file_resume(
    url_ptr: *const c_char,
    path_ptr: *const c_char,
    existing_bytes: u64,
    max_retries: u32,
    out_status: *mut u16,
) -> bool {
    ffi_false_check!(url_ptr, path_ptr);
    let url = ffi_cstr_to_str_false!(url_ptr);
    let path = ffi_cstr_to_str_false!(path_ptr);
    let retries = if max_retries == 0 { None } else { Some(max_retries) };
    match http_client::download_to_file_with_resume(url, path, existing_bytes, retries) {
        Ok(result) => {
            unsafe {
                *out_status = result.status;
            }
            true
        }
        Err(_) => false,
    }
}

#[no_mangle]
pub extern "C" fn launcher_http_post_json(
    url_ptr: *const c_char,
    body_ptr: *const u8,
    body_len: usize,
    out_status: *mut u16,
    out_len: *mut usize,
) -> *mut u8 {
    ffi_null_check!(url_ptr, body_ptr, out_status, out_len);
    let url = ffi_cstr_to_str!(url_ptr);
    let body = unsafe { slice::from_raw_parts(body_ptr, body_len) };
    match http_client::post_json(url, body, None) {
        Ok(response) => {
            unsafe {
                *out_status = response.status;
            }
            ffi_buffer_to_box!(response.body, out_len)
        }
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_settings_new(path_ptr: *const c_char) -> *mut settings::SettingsStore {
    ffi_null_check!(path_ptr);
    let path = ffi_cstr_to_str!(path_ptr);
    Box::into_raw(Box::new(settings::SettingsStore::new(path)))
}

#[no_mangle]
pub extern "C" fn launcher_settings_free(ptr: *mut settings::SettingsStore) {
    if !ptr.is_null() {
        unsafe {
            let _ = Box::from_raw(ptr);
        }
    }
}

#[no_mangle]
pub extern "C" fn launcher_settings_load(ptr: *mut settings::SettingsStore) -> bool {
    if ptr.is_null() {
        return false;
    }
    let store = unsafe { &mut *ptr };
    store.load().is_ok()
}

#[no_mangle]
pub extern "C" fn launcher_settings_save(ptr: *mut settings::SettingsStore) -> bool {
    if ptr.is_null() {
        return false;
    }
    let store = unsafe { &mut *ptr };
    store.save().is_ok()
}

#[no_mangle]
pub extern "C" fn launcher_settings_get_string(ptr: *const settings::SettingsStore, key_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(key_ptr);
    if ptr.is_null() {
        return std::ptr::null_mut();
    }
    let store = unsafe { &*ptr };
    let key = ffi_cstr_to_str!(key_ptr);
    let value = store.get_string(key);
    if value.is_empty() && !store.contains(key) {
        return std::ptr::null_mut();
    }
    ffi_cstring_to_raw!(value)
}

#[no_mangle]
pub extern "C" fn launcher_settings_get_int(ptr: *const settings::SettingsStore, key_ptr: *const c_char) -> i64 {
    if key_ptr.is_null() {
        return 0;
    }
    if ptr.is_null() {
        return 0;
    }
    let store = unsafe { &*ptr };
    let key = ffi_cstr_to_str!(key_ptr);
    store.get_int(key)
}

#[no_mangle]
pub extern "C" fn launcher_settings_get_bool(ptr: *const settings::SettingsStore, key_ptr: *const c_char) -> bool {
    if key_ptr.is_null() {
        return false;
    }
    if ptr.is_null() {
        return false;
    }
    let store = unsafe { &*ptr };
    let key = ffi_cstr_to_str!(key_ptr);
    store.get_bool(key)
}

#[no_mangle]
pub extern "C" fn launcher_settings_set_string(ptr: *mut settings::SettingsStore, key_ptr: *const c_char, value_ptr: *const c_char) {
    if ptr.is_null() || key_ptr.is_null() || value_ptr.is_null() {
        return;
    }
    let store = unsafe { &mut *ptr };
    let key = ffi_cstr_to_str_void!(key_ptr);
    let value = ffi_cstr_to_str_void!(value_ptr);
    store.set_string(key, value);
}

#[no_mangle]
pub extern "C" fn launcher_settings_set_int(ptr: *mut settings::SettingsStore, key_ptr: *const c_char, value: i64) {
    if ptr.is_null() || key_ptr.is_null() {
        return;
    }
    let store = unsafe { &mut *ptr };
    let key = ffi_cstr_to_str_void!(key_ptr);
    store.set_int(key, value);
}

#[no_mangle]
pub extern "C" fn launcher_settings_set_bool(ptr: *mut settings::SettingsStore, key_ptr: *const c_char, value: bool) {
    if ptr.is_null() || key_ptr.is_null() {
        return;
    }
    let store = unsafe { &mut *ptr };
    let key = ffi_cstr_to_str_void!(key_ptr);
    store.set_bool(key, value);
}

#[no_mangle]
pub extern "C" fn launcher_settings_reset(ptr: *mut settings::SettingsStore, key_ptr: *const c_char) {
    if ptr.is_null() || key_ptr.is_null() {
        return;
    }
    let store = unsafe { &mut *ptr };
    let key = ffi_cstr_to_str_void!(key_ptr);
    store.reset(key);
}

#[no_mangle]
pub extern "C" fn launcher_settings_contains(ptr: *const settings::SettingsStore, key_ptr: *const c_char) -> bool {
    ffi_false_check!(key_ptr);
    if ptr.is_null() {
        return false;
    }
    let store = unsafe { &*ptr };
    let key = ffi_cstr_to_str_false!(key_ptr);
    store.contains(key)
}

#[no_mangle]
pub extern "C" fn launcher_settings_register_default(ptr: *mut settings::SettingsStore, key_ptr: *const c_char, value_ptr: *const c_char) {
    if ptr.is_null() || key_ptr.is_null() || value_ptr.is_null() {
        return;
    }
    let store = unsafe { &mut *ptr };
    let key = ffi_cstr_to_str_void!(key_ptr);
    let value = ffi_cstr_to_str_void!(value_ptr);
    store.register_default(key, value);
}

#[no_mangle]
pub extern "C" fn launcher_settings_register_alias(
    ptr: *mut settings::SettingsStore,
    alias_ptr: *const c_char,
    canonical_ptr: *const c_char,
) {
    if ptr.is_null() || alias_ptr.is_null() || canonical_ptr.is_null() {
        return;
    }
    let store = unsafe { &mut *ptr };
    let alias = ffi_cstr_to_str_void!(alias_ptr);
    let canonical = ffi_cstr_to_str_void!(canonical_ptr);
    store.register_alias(alias, canonical);
}

#[no_mangle]
pub extern "C" fn launcher_settings_is_dirty(ptr: *const settings::SettingsStore) -> bool {
    if ptr.is_null() {
        return false;
    }
    let store = unsafe { &*ptr };
    store.is_dirty()
}

#[no_mangle]
pub extern "C" fn launcher_settings_keys(ptr: *const settings::SettingsStore, out_count: *mut usize) -> *mut *mut c_char {
    ffi_null_check!(out_count);
    if ptr.is_null() {
        unsafe {
            *out_count = 0;
        }
        return std::ptr::null_mut();
    }
    let store = unsafe { &*ptr };
    let keys: Vec<String> = store.keys().into_iter().map(String::from).collect();
    ffi_string_vec_to_raw!(keys, out_count)
}

#[no_mangle]
pub extern "C" fn launcher_parse_modrinth_project(json_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(json_ptr);
    let json = ffi_cstr_to_str!(json_ptr);
    match mod_metadata::parse_modrinth_project(json) {
        Ok(project) => {
            let serialized = serde_json::to_string(&project).unwrap_or_default();
            ffi_cstring_to_raw!(serialized)
        }
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_parse_modrinth_version(json_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(json_ptr);
    let json = ffi_cstr_to_str!(json_ptr);
    match mod_metadata::parse_modrinth_version(json) {
        Ok(version) => {
            let serialized = serde_json::to_string(&version).unwrap_or_default();
            ffi_cstring_to_raw!(serialized)
        }
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_parse_curseforge_project(json_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(json_ptr);
    let json = ffi_cstr_to_str!(json_ptr);
    match mod_metadata::parse_curseforge_project(json) {
        Ok(project) => {
            let serialized = serde_json::to_string(&project).unwrap_or_default();
            ffi_cstring_to_raw!(serialized)
        }
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
pub extern "C" fn launcher_parse_curseforge_version(json_ptr: *const c_char) -> *mut c_char {
    ffi_null_check!(json_ptr);
    let json = ffi_cstr_to_str!(json_ptr);
    match mod_metadata::parse_curseforge_version(json) {
        Ok(version) => {
            let serialized = serde_json::to_string(&version).unwrap_or_default();
            ffi_cstring_to_raw!(serialized)
        }
        Err(_) => std::ptr::null_mut(),
    }
}
