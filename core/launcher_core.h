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

#ifdef __cplusplus
}
#endif
