use std::fs;
use std::path::Path;

use error::Result;

pub fn read(path: impl AsRef<Path>) -> Result<Vec<u8>> {
    Ok(fs::read(path)?)
}

pub fn read_string(path: impl AsRef<Path>) -> Result<String> {
    Ok(fs::read_to_string(path)?)
}

pub fn write(path: impl AsRef<Path>, content_bytes: &[u8]) -> Result<()> {
    if let Some(parent) = path.as_ref().parent() {
        fs::create_dir_all(parent)?;
    }
    fs::write(path, content_bytes)?;
    Ok(())
}

pub fn write_string(path: impl AsRef<Path>, content_text: &str) -> Result<()> {
    write(path, content_text.as_bytes())
}

pub fn append(path: impl AsRef<Path>, content_bytes: &[u8]) -> Result<()> {
    use std::io::Write;
    let mut file = fs::OpenOptions::new()
        .create(true)
        .append(true)
        .open(path)?;
    file.write_all(content_bytes)?;
    Ok(())
}

pub fn append_safe(path: impl AsRef<Path>, content_bytes: &[u8]) -> Result<()> {
    use std::io::Write;
    let mut file = fs::OpenOptions::new()
        .create(true)
        .append(true)
        .truncate(false)
        .open(path)?;
    file.write_all(content_bytes)?;
    Ok(())
}

pub fn ensure_dir_exists(path: impl AsRef<Path>) -> Result<()> {
    fs::create_dir_all(path)?;
    Ok(())
}

pub fn ensure_parent_exists(path: impl AsRef<Path>) -> Result<()> {
    if let Some(parent) = path.as_ref().parent() {
        fs::create_dir_all(parent)?;
    }
    Ok(())
}

pub fn delete_path(path: impl AsRef<Path>) -> Result<()> {
    let path = path.as_ref();
    if path.is_dir() {
        fs::remove_dir_all(path)?;
    } else {
        fs::remove_file(path)?;
    }
    Ok(())
}

pub fn delete_contents(path: impl AsRef<Path>) -> Result<()> {
    for entry in fs::read_dir(path)? {
        let entry = entry?;
        let path = entry.path();
        if path.is_dir() {
            fs::remove_dir_all(&path)?;
        } else {
            fs::remove_file(&path)?;
        }
    }
    Ok(())
}

pub fn move_file(source: impl AsRef<Path>, dest: impl AsRef<Path>) -> Result<()> {
    fs::rename(source, dest)?;
    Ok(())
}

pub fn path_combine(parts: &[&str]) -> String {
    parts.join(std::path::MAIN_SEPARATOR_STR.to_string().as_str())
}

pub fn normalize_path(path: &str) -> String {
    path.replace('\\', "/")
}

pub fn remove_invalid_filename_chars(input_string: &str, replace_with: char) -> String {
    input_string
        .chars()
        .map(|c| {
            if c == '<'
                || c == '>'
                || c == ':'
                || c == '"'
                || c == '/'
                || c == '\\'
                || c == '|'
                || c == '?'
                || c == '*'
            {
                replace_with
            } else {
                c
            }
        })
        .collect()
}

pub fn remove_invalid_path_chars(input_string: &str, replace_with: char) -> String {
    input_string
        .chars()
        .map(|c| {
            if c == '<' || c == '>' || c == '"' || c == '|' || c == '?' || c == '*' {
                replace_with
            } else {
                c
            }
        })
        .collect()
}

pub fn dir_name_from_string(s: &str, in_dir: &str) -> String {
    let mut name = s.to_string();
    let mut counter = 1;
    let path = Path::new(in_dir).join(&name);
    if !path.exists() {
        return name;
    }
    loop {
        name = format!("{} ({})", s, counter);
        let path = Path::new(in_dir).join(&name);
        if !path.exists() {
            return name;
        }
        counter += 1;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;

    #[test]
    fn read_write_roundtrip() {
        let dir = std::env::temp_dir().join("core_test");
        fs::create_dir_all(&dir).unwrap();
        let path = dir.join("test.txt");
        write(&path, b"hello").unwrap();
        assert_eq!(read(&path).unwrap(), b"hello");
        fs::remove_dir_all(&dir).unwrap();
    }

    #[test]
    fn remove_invalid_chars() {
        assert_eq!(remove_invalid_filename_chars("a<b>c", '_'), "a_b_c");
        assert_eq!(remove_invalid_path_chars("a|b?c", '_'), "a_b_c");
    }
}
