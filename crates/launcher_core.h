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

#ifdef __cplusplus
}
#endif
