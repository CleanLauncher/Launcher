use std::collections::HashMap;
use std::fs;
use std::path::{Path, PathBuf};

use error::{CoreError, Result};

pub struct SettingsStore {
    file_path: PathBuf,
    values: HashMap<String, String>,
    defaults: HashMap<String, String>,
    aliases: HashMap<String, String>,
    dirty: bool,
}

impl SettingsStore {
    pub fn new(file_path: &str) -> Self {
        SettingsStore {
            file_path: PathBuf::from(file_path),
            values: HashMap::new(),
            defaults: HashMap::new(),
            aliases: HashMap::new(),
            dirty: false,
        }
    }

    pub fn load(&mut self) -> Result<()> {
        if !self.file_path.exists() {
            return Ok(());
        }
        let content = fs::read_to_string(&self.file_path).map_err(CoreError::Io)?;
        self.values = parse_ini(&content);
        self.dirty = false;
        Ok(())
    }

    pub fn save(&mut self) -> Result<()> {
        if !self.dirty {
            return Ok(());
        }
        let content = serialize_ini(&self.values);
        let parent = self.file_path.parent().unwrap_or(Path::new("."));
        fs::create_dir_all(parent).map_err(CoreError::Io)?;
        let temp_path = self.file_path.with_extension("tmp");
        fs::write(&temp_path, content).map_err(CoreError::Io)?;
        fs::rename(&temp_path, &self.file_path).map_err(CoreError::Io)?;
        self.dirty = false;
        Ok(())
    }

    pub fn register_default(&mut self, key: &str, default_value: &str) {
        self.defaults.insert(key.to_string(), default_value.to_string());
    }

    pub fn register_alias(&mut self, alias: &str, canonical: &str) {
        self.aliases.insert(alias.to_string(), canonical.to_string());
    }

    fn resolve_key<'a>(&'a self, key: &'a str) -> &'a str {
        self.aliases.get(key).map(|s| s.as_str()).unwrap_or(key)
    }

    pub fn get_string(&self, key: &str) -> String {
        let canonical = self.resolve_key(key);
        self.values
            .get(canonical)
            .cloned()
            .or_else(|| self.defaults.get(canonical).cloned())
            .unwrap_or_default()
    }

    pub fn get_int(&self, key: &str) -> i64 {
        let canonical = self.resolve_key(key);
        let raw = self.values.get(canonical).or_else(|| self.defaults.get(canonical));
        match raw {
            Some(v) => v.parse().unwrap_or(0),
            None => 0,
        }
    }

    pub fn get_bool(&self, key: &str) -> bool {
        let canonical = self.resolve_key(key);
        let raw = self.values.get(canonical).or_else(|| self.defaults.get(canonical));
        match raw {
            Some(v) => matches!(v.as_str(), "true" | "1" | "yes"),
            None => false,
        }
    }

    pub fn get_double(&self, key: &str) -> f64 {
        let canonical = self.resolve_key(key);
        let raw = self.values.get(canonical).or_else(|| self.defaults.get(canonical));
        match raw {
            Some(v) => v.parse().unwrap_or(0.0),
            None => 0.0,
        }
    }

    pub fn set_string(&mut self, key: &str, value: &str) {
        let canonical = self.resolve_key(key);
        self.values.insert(canonical.to_string(), value.to_string());
        self.dirty = true;
    }

    pub fn set_int(&mut self, key: &str, value: i64) {
        let canonical = self.resolve_key(key);
        self.values.insert(canonical.to_string(), value.to_string());
        self.dirty = true;
    }

    pub fn set_bool(&mut self, key: &str, value: bool) {
        let canonical = self.resolve_key(key);
        self.values.insert(canonical.to_string(), value.to_string());
        self.dirty = true;
    }

    pub fn set_double(&mut self, key: &str, value: f64) {
        let canonical = self.resolve_key(key);
        self.values.insert(canonical.to_string(), format!("{value}"));
        self.dirty = true;
    }

    pub fn reset(&mut self, key: &str) {
        let canonical = self.resolve_key(key).to_string();
        self.values.remove(&canonical);
        self.dirty = true;
    }

    pub fn contains(&self, key: &str) -> bool {
        let canonical = self.resolve_key(key);
        self.values.contains_key(canonical)
    }

    pub fn get_or_set_string(&mut self, key: &str, default_value: &str) -> String {
        let canonical = self.resolve_key(key).to_string();
        if let Some(v) = self.values.get(&canonical) {
            return v.clone();
        }
        self.values.insert(canonical, default_value.to_string());
        self.dirty = true;
        default_value.to_string()
    }

    pub fn get_or_set_int(&mut self, key: &str, default_value: i64) -> i64 {
        let canonical = self.resolve_key(key).to_string();
        if let Some(v) = self.values.get(&canonical) {
            return v.parse().unwrap_or(default_value);
        }
        self.values.insert(canonical, default_value.to_string());
        self.dirty = true;
        default_value
    }

    pub fn get_or_set_bool(&mut self, key: &str, default_value: bool) -> bool {
        let canonical = self.resolve_key(key).to_string();
        if let Some(v) = self.values.get(&canonical) {
            return matches!(v.as_str(), "true" | "1" | "yes");
        }
        self.values.insert(canonical, default_value.to_string());
        self.dirty = true;
        default_value
    }

    pub fn is_dirty(&self) -> bool {
        self.dirty
    }

    pub fn keys(&self) -> Vec<&str> {
        self.values.keys().map(|s| s.as_str()).collect()
    }

    pub fn clear(&mut self) {
        self.values.clear();
        self.dirty = true;
    }
}

pub fn parse_ini(content: &str) -> HashMap<String, String> {
    let mut map = HashMap::new();
    let mut current_section = String::new();
    for line in content.lines() {
        let trimmed = line.trim();
        if trimmed.is_empty() || trimmed.starts_with('#') || trimmed.starts_with(';') {
            continue;
        }
        if trimmed.starts_with('[') && trimmed.ends_with(']') {
            current_section = trimmed[1..trimmed.len() - 1].trim().to_string();
            continue;
        }
        if let Some(eq_pos) = trimmed.find('=') {
            let key = trimmed[..eq_pos].trim();
            let value = trimmed[eq_pos + 1..].trim();
            let full_key = if current_section.is_empty() {
                key.to_string()
            } else {
                format!("{current_section}/{key}")
            };
            map.insert(full_key, unescape_ini_value(value));
        }
    }
    map
}

fn unescape_ini_value(value: &str) -> String {
    let mut result = String::with_capacity(value.len());
    let mut chars = value.chars();
    while let Some(c) = chars.next() {
        if c == '\\' {
            match chars.next() {
                Some('n') => result.push('\n'),
                Some('t') => result.push('\t'),
                Some('#') => result.push('#'),
                Some('\\') => result.push('\\'),
                Some(other) => {
                    result.push('\\');
                    result.push(other);
                }
                None => result.push('\\'),
            }
        } else {
            result.push(c);
        }
    }
    result
}

fn serialize_ini(values: &HashMap<String, String>) -> String {
    let mut sections: HashMap<String, Vec<(String, String)>> = HashMap::new();
    let mut root_keys: Vec<(String, String)> = Vec::new();
    for (key, value) in values {
        if let Some(slash_pos) = key.find('/') {
            let section = &key[..slash_pos];
            let sub_key = &key[slash_pos + 1..];
            sections
                .entry(section.to_string())
                .or_default()
                .push((sub_key.to_string(), value.clone()));
        } else {
            root_keys.push((key.clone(), value.clone()));
        }
    }
    let mut output = String::new();
    for (key, value) in &root_keys {
        let escaped = escape_ini_value(value);
        output.push_str(&format!("{key}={escaped}\n"));
    }
    let mut sorted_sections: Vec<_> = sections.into_iter().collect();
    sorted_sections.sort_by(|a, b| a.0.cmp(&b.0));
    for (section, keys) in sorted_sections {
        output.push_str(&format!("\n[{section}]\n"));
        let mut sorted_keys = keys;
        sorted_keys.sort_by(|a, b| a.0.cmp(&b.0));
        for (key, value) in &sorted_keys {
            let escaped = escape_ini_value(value);
            output.push_str(&format!("{key}={escaped}\n"));
        }
    }
    output
}

fn escape_ini_value(value: &str) -> String {
    let mut result = String::with_capacity(value.len());
    for c in value.chars() {
        match c {
            '\n' => result.push_str("\\n"),
            '\t' => result.push_str("\\t"),
            '\\' => result.push_str("\\\\"),
            _ => result.push(c),
        }
    }
    result
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_ini_simple() {
        let content = "name=Test\nversion=1.0\n";
        let map = parse_ini(content);
        assert_eq!(map.get("name").unwrap(), "Test");
        assert_eq!(map.get("version").unwrap(), "1.0");
    }

    #[test]
    fn test_parse_ini_sections() {
        let content = "[General]\nname=Test\n[Network]\nproxy=none\n";
        let map = parse_ini(content);
        assert_eq!(map.get("General/name").unwrap(), "Test");
        assert_eq!(map.get("Network/proxy").unwrap(), "none");
    }

    #[test]
    fn test_parse_ini_comments() {
        let content = "# comment\nname=Test\n; another\nversion=1.0\n";
        let map = parse_ini(content);
        assert_eq!(map.len(), 2);
        assert_eq!(map.get("name").unwrap(), "Test");
    }

    #[test]
    fn test_parse_ini_escaping() {
        let content = "path=C:\\Users\\test\nmulti=line1\\nline2\n";
        let map = parse_ini(content);
        assert_eq!(map.get("path").unwrap(), "C:\\Users\\test");
        assert_eq!(map.get("multi").unwrap(), "line1\nline2");
    }

    #[test]
    fn test_serialize_roundtrip() {
        let mut values = HashMap::new();
        values.insert("name".to_string(), "Test".to_string());
        values.insert("General/version".to_string(), "1.0".to_string());
        let serialized = serialize_ini(&values);
        let parsed = parse_ini(&serialized);
        assert_eq!(parsed.get("name").unwrap(), "Test");
        assert_eq!(parsed.get("General/version").unwrap(), "1.0");
    }

    #[test]
    fn test_store_defaults() {
        let mut store = SettingsStore::new("/tmp/test_settings.ini");
        store.register_default("name", "DefaultName");
        assert_eq!(store.get_string("name"), "DefaultName");
        assert_eq!(store.get_int("missing"), 0);
        assert!(!store.get_bool("missing"));
    }

    #[test]
    fn test_store_set_get() {
        let mut store = SettingsStore::new("/tmp/test_settings.ini");
        store.set_string("name", "Hello");
        store.set_int("count", 42);
        store.set_bool("flag", true);
        assert_eq!(store.get_string("name"), "Hello");
        assert_eq!(store.get_int("count"), 42);
        assert!(store.get_bool("flag"));
        assert!(store.is_dirty());
    }

    #[test]
    fn test_store_aliases() {
        let mut store = SettingsStore::new("/tmp/test_settings.ini");
        store.register_alias("OldName", "NewName");
        store.set_string("OldName", "value");
        assert_eq!(store.get_string("NewName"), "value");
        assert!(store.contains("NewName"));
    }

    #[test]
    fn test_store_get_or_set() {
        let mut store = SettingsStore::new("/tmp/test_settings.ini");
        let val = store.get_or_set_string("key", "default");
        assert_eq!(val, "default");
        assert!(store.is_dirty());
        assert_eq!(store.get_string("key"), "default");
    }

    #[test]
    fn test_store_reset() {
        let mut store = SettingsStore::new("/tmp/test_settings.ini");
        store.set_string("key", "value");
        store.reset("key");
        assert!(!store.contains("key"));
    }

    #[test]
    fn test_store_save_load() {
        let path = "/tmp/test_settings_rw.ini";
        let _ = fs::remove_file(path);
        {
            let mut store = SettingsStore::new(path);
            store.set_string("name", "Test");
            store.set_int("count", 42);
            store.save().unwrap();
        }
        {
            let mut store = SettingsStore::new(path);
            store.load().unwrap();
            assert_eq!(store.get_string("name"), "Test");
            assert_eq!(store.get_int("count"), 42);
        }
        let _ = fs::remove_file(path);
    }

    #[test]
    fn test_store_save_no_change() {
        let path = "/tmp/test_settings_nochange.ini";
        let _ = fs::remove_file(path);
        {
            let mut store = SettingsStore::new(path);
            store.save().unwrap();
            assert!(!Path::new(path).exists());
        }
        let _ = fs::remove_file(path);
    }
}
