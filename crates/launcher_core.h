#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t* launcher_gzip_unzip(const uint8_t* compressed, size_t compressed_len, size_t* out_len);
uint8_t* launcher_gzip_zip(const uint8_t* uncompressed, size_t uncompressed_len, size_t* out_len);
void launcher_free_buffer(uint8_t* ptr, size_t len);
char* launcher_markdown_to_html(const char* markdown);
void launcher_free_string(char* ptr);
int launcher_natural_compare(const char* s1, const char* s2, bool case_insensitive);
char* launcher_human_readable_file_size(double bytes, bool use_si, size_t decimal_points);
bool launcher_json_validate(const uint8_t* data, size_t len);
bool launcher_json_is_binary(const uint8_t* data, size_t len);
uint8_t* launcher_fs_read(const char* path, size_t* out_len);
bool launcher_fs_write(const char* path, const uint8_t* data, size_t len);
bool launcher_fs_delete_path(const char* path);
char* launcher_fs_remove_invalid_filename_chars(const char* input, char replace_with);
char* launcher_hash_sha256(const uint8_t* data, size_t len);
char* launcher_hash_sha512(const uint8_t* data, size_t len);
char* launcher_hash_md5(const uint8_t* data, size_t len);
char* launcher_hash_sha256_file(const char* path);
bool launcher_verify_sha256(const uint8_t* data, size_t len, const char* expected);
char** launcher_zip_entry_names(const char* archive_path, size_t* out_count);
void launcher_free_string_list(char** ptr, size_t count);
uint8_t* launcher_zip_read_entry(const char* archive_path, const char* entry_name, size_t* out_len);
bool launcher_zip_extract_file(const char* archive_path, const char* entry_name, const char* target_path);
char** launcher_zip_extract_dir(const char* archive_path, const char* subdir_prefix, const char* target_dir, size_t* out_count);
bool launcher_zip_entry_exists(const char* archive_path, const char* entry_name);
char** launcher_tar_entry_names(const char* archive_path, size_t* out_count);
char** launcher_tar_extract_dir(const char* archive_path, const char* target_dir, size_t* out_count);

void launcher_http_set_user_agent(const char* agent);
void launcher_http_set_timeout(uint64_t timeout_ms);
void launcher_http_set_header(const char* name, const char* value);
uint8_t* launcher_http_get(const char* url, uint16_t* out_status, size_t* out_len);
bool launcher_http_get_file(const char* url, const char* path, uint16_t* out_status);
bool launcher_http_get_file_resume(const char* url, const char* path, uint64_t existing_bytes, uint32_t max_retries, uint16_t* out_status);
uint8_t* launcher_http_post_json(const char* url, const uint8_t* body, size_t body_len, uint16_t* out_status, size_t* out_len);

typedef struct SettingsStore SettingsStore;
SettingsStore* launcher_settings_new(const char* path);
void launcher_settings_free(SettingsStore* ptr);
bool launcher_settings_load(SettingsStore* ptr);
bool launcher_settings_save(SettingsStore* ptr);
char* launcher_settings_get_string(const SettingsStore* ptr, const char* key);
int64_t launcher_settings_get_int(const SettingsStore* ptr, const char* key);
bool launcher_settings_get_bool(const SettingsStore* ptr, const char* key);
void launcher_settings_set_string(SettingsStore* ptr, const char* key, const char* value);
void launcher_settings_set_int(SettingsStore* ptr, const char* key, int64_t value);
void launcher_settings_set_bool(SettingsStore* ptr, const char* key, bool value);
void launcher_settings_reset(SettingsStore* ptr, const char* key);
bool launcher_settings_contains(const SettingsStore* ptr, const char* key);
void launcher_settings_register_default(SettingsStore* ptr, const char* key, const char* value);
void launcher_settings_register_alias(SettingsStore* ptr, const char* alias, const char* canonical);
bool launcher_settings_is_dirty(const SettingsStore* ptr);
char** launcher_settings_keys(const SettingsStore* ptr, size_t* out_count);

char* launcher_parse_modrinth_project(const char* json);
char* launcher_parse_modrinth_version(const char* json);
char* launcher_parse_curseforge_project(const char* json);
char* launcher_parse_curseforge_version(const char* json);

#ifdef __cplusplus
}
#endif
