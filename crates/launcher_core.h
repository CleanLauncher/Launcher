// launcher-core FFI bindings
// Auto-generated from Rust launcher-core library
#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// GZip decompression
// Returns null on error. Caller must free with launcher_free_buffer.
uint8_t* launcher_gzip_unzip(const uint8_t* compressed, size_t compressed_len, size_t* out_len);

// GZip compression
// Returns null on error. Caller must free with launcher_free_buffer.
uint8_t* launcher_gzip_zip(const uint8_t* uncompressed, size_t uncompressed_len, size_t* out_len);

// Free a buffer returned by launcher_gzip_* functions.
void launcher_free_buffer(uint8_t* ptr, size_t len);

// Convert markdown to HTML.
// Returns null on error. Caller must free with launcher_free_string.
char* launcher_markdown_to_html(const char* markdown);

// Free a string returned by launcher_markdown_to_html or other functions.
void launcher_free_string(char* ptr);

// Natural compare two strings. Returns -1, 0, or 1.
int launcher_natural_compare(const char* s1, const char* s2, bool case_insensitive);

// Format file size as human readable string.
// Caller must free with launcher_free_string.
char* launcher_human_readable_file_size(double bytes, bool use_si, size_t decimal_points);

// Validate JSON data. Returns true if valid.
bool launcher_json_validate(const uint8_t* data, size_t len);

// Check if data is binary JSON format.
bool launcher_json_is_binary(const uint8_t* data, size_t len);

// Read file contents. Returns null on error. Caller must free with launcher_free_buffer.
uint8_t* launcher_fs_read(const char* path, size_t* out_len);

// Write data to file. Returns true on success.
bool launcher_fs_write(const char* path, const uint8_t* data, size_t len);

// Delete path. Returns true on success.
bool launcher_fs_delete_path(const char* path);

// Remove invalid filename characters.
// Caller must free with launcher_free_string.
char* launcher_fs_remove_invalid_filename_chars(const char* input, char replace_with);

// Compute SHA-256 hash of data. Returns hex string. Caller must free with launcher_free_string.
char* launcher_hash_sha256(const uint8_t* data, size_t len);

// Compute SHA-512 hash of data. Returns hex string. Caller must free with launcher_free_string.
char* launcher_hash_sha512(const uint8_t* data, size_t len);

// Compute MD5 hash of data. Returns hex string. Caller must free with launcher_free_string.
char* launcher_hash_md5(const uint8_t* data, size_t len);

// Compute SHA-256 hash of a file. Returns hex string. Caller must free with launcher_free_string.
char* launcher_hash_sha256_file(const char* path);

// Verify SHA-256 hash of data against expected hash. Returns true on match.
bool launcher_verify_sha256(const uint8_t* data, size_t len, const char* expected);

// List all entry names in a ZIP archive.
// Returns null on error. Caller must free with launcher_free_string_list.
char** launcher_zip_entry_names(const char* archive_path, size_t* out_count);

// Free a string list returned by launcher_zip_entry_names or launcher_zip_extract_dir.
void launcher_free_string_list(char** ptr, size_t count);

// Read a single entry from a ZIP archive into memory.
// Returns null on error. Caller must free with launcher_free_buffer.
uint8_t* launcher_zip_read_entry(const char* archive_path, const char* entry_name, size_t* out_len);

// Extract a single file from a ZIP archive to disk. Returns true on success.
bool launcher_zip_extract_file(const char* archive_path, const char* entry_name, const char* target_path);

// Extract a directory from a ZIP archive to disk. Returns extracted file count, or -1 on error.
// Caller must free result names with launcher_free_string_list.
char** launcher_zip_extract_dir(const char* archive_path, const char* subdir_prefix, const char* target_dir, size_t* out_count);

// Check if an entry exists in a ZIP archive. Returns true if found.
bool launcher_zip_entry_exists(const char* archive_path, const char* entry_name);

// List entries in a TAR.GZ archive.
// Returns null on error. Caller must free with launcher_free_string_list.
char** launcher_tar_entry_names(const char* archive_path, size_t* out_count);

// Extract a TAR.GZ archive to disk. Returns extracted file count, or -1 on error.
// Caller must free result names with launcher_free_string_list.
char** launcher_tar_extract_dir(const char* archive_path, const char* target_dir, size_t* out_count);

// --- HTTP Client ---

// Set the global HTTP user agent string.
void launcher_http_set_user_agent(const char* agent);

// Set the global HTTP request timeout in milliseconds.
void launcher_http_set_timeout(uint64_t timeout_ms);

// Set a custom HTTP header for all subsequent requests.
void launcher_http_set_header(const char* name, const char* value);

// Perform an HTTP GET request. Returns response body.
// out_status receives the HTTP status code.
// Returns null on error. Caller must free with launcher_free_buffer.
uint8_t* launcher_http_get(const char* url, uint16_t* out_status, size_t* out_len);

// Download a URL to a file. Returns true on success.
// out_status receives the HTTP status code.
bool launcher_http_get_file(const char* url, const char* path, uint16_t* out_status);

// Download a URL to a file with resume support. Pass the existing file size to resume from.
// Returns true on success. out_status receives the HTTP status code.
bool launcher_http_get_file_resume(const char* url, const char* path, uint64_t existing_bytes, uint32_t max_retries, uint16_t* out_status);

// Perform an HTTP POST request with a JSON body. Returns response body.
// out_status receives the HTTP status code.
// Returns null on error. Caller must free with launcher_free_buffer.
uint8_t* launcher_http_post_json(const char* url, const uint8_t* body, size_t body_len, uint16_t* out_status, size_t* out_len);

// --- Settings ---

// Opaque handle to a settings store.
typedef struct SettingsStore SettingsStore;

// Create a new settings store for the given file path.
// Returns null on error. Caller must free with launcher_settings_free.
SettingsStore* launcher_settings_new(const char* path);

// Free a settings store.
void launcher_settings_free(SettingsStore* ptr);

// Load settings from the file. Returns true on success.
bool launcher_settings_load(SettingsStore* ptr);

// Save settings to the file. Returns true on success.
bool launcher_settings_save(SettingsStore* ptr);

// Get a string value. Returns null if key is missing and no default is set.
// Caller must free with launcher_free_string.
char* launcher_settings_get_string(const SettingsStore* ptr, const char* key);

// Get an integer value. Returns 0 if key is missing.
int64_t launcher_settings_get_int(const SettingsStore* ptr, const char* key);

// Get a boolean value. Returns false if key is missing.
bool launcher_settings_get_bool(const SettingsStore* ptr, const char* key);

// Set a string value.
void launcher_settings_set_string(SettingsStore* ptr, const char* key, const char* value);

// Set an integer value.
void launcher_settings_set_int(SettingsStore* ptr, const char* key, int64_t value);

// Set a boolean value.
void launcher_settings_set_bool(SettingsStore* ptr, const char* key, bool value);

// Reset a setting to its default (removes the key from storage).
void launcher_settings_reset(SettingsStore* ptr, const char* key);

// Check if a key exists in the store.
bool launcher_settings_contains(const SettingsStore* ptr, const char* key);

// Register a default value for a key.
void launcher_settings_register_default(SettingsStore* ptr, const char* key, const char* value);

// Register an alias: when alias is used, it resolves to canonical.
void launcher_settings_register_alias(SettingsStore* ptr, const char* alias, const char* canonical);

// Check if the store has unsaved changes.
bool launcher_settings_is_dirty(const SettingsStore* ptr);

// Get all keys. Caller must free with launcher_free_string_list.
char** launcher_settings_keys(const SettingsStore* ptr, size_t* out_count);

#ifdef __cplusplus
}
#endif
