use std::collections::HashSet;
use std::fs;
use std::io::{Read, Write};
use std::path::PathBuf;

use flate2::read::GzDecoder;
use tar::Archive as TarArchive;
use zip::read::ZipArchive;
use zip::write::SimpleFileOptions;
use zip::ZipWriter;

use error::{CoreError, Result};

pub struct ZipEntryInfo {
    pub entry_name: String,
    pub is_directory: bool,
    pub uncompressed_size: u64,
    pub compressed_size: u64,
}

pub fn zip_list_entries(archive_path: &str) -> Result<Vec<ZipEntryInfo>> {
    let file_handle = fs::File::open(archive_path)?;
    let mut zip_archive = ZipArchive::new(file_handle)?;

    let mut entry_list = Vec::with_capacity(zip_archive.len());
    for index in 0..zip_archive.len() {
        let zip_file = zip_archive.by_index(index)?;
        entry_list.push(ZipEntryInfo {
            entry_name: zip_file.name().to_string(),
            is_directory: zip_file.is_dir(),
            uncompressed_size: zip_file.size(),
            compressed_size: zip_file.compressed_size(),
        });
    }
    Ok(entry_list)
}

pub fn zip_entry_exists(archive_path: &str, entry_name: &str) -> Result<bool> {
    let file_handle = fs::File::open(archive_path)?;
    let mut zip_archive = ZipArchive::new(file_handle)?;
    let found = zip_archive.by_name(entry_name).is_ok();
    Ok(found)
}

pub fn zip_read_entry(archive_path: &str, entry_name: &str) -> Result<Vec<u8>> {
    let file_handle = fs::File::open(archive_path)?;
    let mut zip_archive = ZipArchive::new(file_handle)?;
    let mut zip_file = zip_archive.by_name(entry_name)?;
    let mut entry_buffer = Vec::with_capacity(zip_file.size() as usize);
    zip_file.read_to_end(&mut entry_buffer)?;
    Ok(entry_buffer)
}

pub fn zip_extract_file(archive_path: &str, entry_name: &str, target_path: &str) -> Result<()> {
    validate_path_safety(entry_name)?;

    let file_handle = fs::File::open(archive_path)?;
    let mut zip_archive = ZipArchive::new(file_handle)?;
    let mut zip_file = zip_archive.by_name(entry_name)?;

    let target_file_path = PathBuf::from(target_path);
    if let Some(parent_directory) = target_file_path.parent() {
        fs::create_dir_all(parent_directory)?;
    }
    let mut output_file = fs::File::create(&target_file_path)?;
    std::io::copy(&mut zip_file, &mut output_file)?;
    Ok(())
}

pub fn zip_extract_dir(archive_path: &str, subdir_prefix: &str, target_dir: &str) -> Result<Vec<String>> {
    let file_handle = fs::File::open(archive_path)?;
    let mut zip_archive = ZipArchive::new(file_handle)?;

    let mut extracted_files = Vec::new();
    let target_root = PathBuf::from(target_dir);
    fs::create_dir_all(&target_root)?;

    for index in 0..zip_archive.len() {
        let mut zip_file = zip_archive.by_index(index)?;

        let entry_full_name = zip_file.name().to_string();
        let entry_relative_path = if subdir_prefix.is_empty() {
            entry_full_name.clone()
        } else if let Some(stripped) = entry_full_name.strip_prefix(subdir_prefix) {
            stripped.to_string()
        } else {
            continue;
        };

        if entry_relative_path.is_empty() {
            continue;
        }
        validate_path_safety(&entry_relative_path)?;

        let output_path = target_root.join(&entry_relative_path);

        if zip_file.is_dir() {
            fs::create_dir_all(&output_path)?;
            continue;
        }

        if let Some(parent_directory) = output_path.parent() {
            fs::create_dir_all(parent_directory)?;
        }
        let mut output_file = fs::File::create(&output_path)?;
        std::io::copy(&mut zip_file, &mut output_file)?;
        extracted_files.push(entry_full_name);
    }
    Ok(extracted_files)
}

pub fn zip_entry_names(archive_path: &str) -> Result<Vec<String>> {
    let file_handle = fs::File::open(archive_path)?;
    let mut zip_archive = ZipArchive::new(file_handle)?;

    let mut name_list = Vec::with_capacity(zip_archive.len());
    for index in 0..zip_archive.len() {
        let zip_file = zip_archive.by_index(index)?;
        name_list.push(zip_file.name().to_string());
    }
    Ok(name_list)
}

pub fn zip_create_from_entries(archive_path: &str, entries: &[(String, Vec<u8>)]) -> Result<()> {
    let output_file = fs::File::create(archive_path)?;
    let mut zip_writer = ZipWriter::new(output_file);
    let write_options = SimpleFileOptions::default()
        .compression_method(zip::CompressionMethod::Deflated)
        .compression_level(Some(6));

    for (entry_name, entry_data) in entries {
        if entry_name.ends_with('/') {
            zip_writer.add_directory(entry_name, write_options)?;
        } else {
            zip_writer.start_file(entry_name, write_options)?;
            zip_writer.write_all(entry_data)?;
        }
    }

    zip_writer.finish()?;
    Ok(())
}

pub fn zip_merge_archives(source_paths: &[&str], target_path: &str, exclude_set: &HashSet<String>) -> Result<()> {
    let output_file = fs::File::create(target_path)?;
    let mut zip_writer = ZipWriter::new(output_file);
    let write_options = SimpleFileOptions::default()
        .compression_method(zip::CompressionMethod::Deflated)
        .compression_level(Some(6));

    for source_path in source_paths {
        let source_file = fs::File::open(source_path)?;
        let mut source_archive = ZipArchive::new(source_file)?;

        for index in 0..source_archive.len() {
            let mut source_entry = source_archive.by_index(index)?;
            let entry_name = source_entry.name().to_string();

            if exclude_set.contains(&entry_name) {
                continue;
            }

            if source_entry.is_dir() {
                zip_writer.add_directory(&entry_name, write_options)?;
            } else {
                let mut entry_buffer = Vec::with_capacity(source_entry.size() as usize);
                source_entry.read_to_end(&mut entry_buffer)?;
                zip_writer.start_file(&entry_name, write_options)?;
                zip_writer.write_all(&entry_buffer)?;
            }
        }
    }

    zip_writer.finish()?;
    Ok(())
}

pub fn tar_list_entries(archive_path: &str) -> Result<Vec<ZipEntryInfo>> {
    let file_handle = fs::File::open(archive_path)?;
    let gz_decoder = GzDecoder::new(file_handle);
    let mut tar_archive = TarArchive::new(gz_decoder);

    let mut entry_list = Vec::new();
    let tar_entries = tar_archive.entries()?;

    for tar_entry_result in tar_entries {
        let tar_entry = tar_entry_result?;
        let entry_path = tar_entry.path()?.into_owned();
        let entry_header = tar_entry.header();

        let entry_size = entry_header.size()?;
        let is_directory = entry_header.entry_type() == tar::EntryType::Directory;
        entry_list.push(ZipEntryInfo {
            entry_name: entry_path.to_string_lossy().to_string(),
            is_directory,
            uncompressed_size: entry_size,
            compressed_size: entry_size,
        });
    }
    Ok(entry_list)
}

pub fn tar_extract_dir(archive_path: &str, target_dir: &str) -> Result<Vec<String>> {
    let file_handle = fs::File::open(archive_path)?;
    let gz_decoder = GzDecoder::new(file_handle);
    let mut tar_archive = TarArchive::new(gz_decoder);

    let target_root = PathBuf::from(target_dir);
    fs::create_dir_all(&target_root)?;

    let mut extracted_files = Vec::new();
    let tar_entries = tar_archive.entries()?;

    for tar_entry_result in tar_entries {
        let mut tar_entry = tar_entry_result?;

        let entry_path = tar_entry.path()?.into_owned();

        let entry_path_string = entry_path.to_string_lossy().to_string();
        validate_path_safety(&entry_path_string)?;

        let output_path = target_root.join(&entry_path);

        if tar_entry.header().entry_type() == tar::EntryType::Directory {
            fs::create_dir_all(&output_path)?;
            continue;
        }

        if let Some(parent_directory) = output_path.parent() {
            fs::create_dir_all(parent_directory)?;
        }
        tar_entry.unpack_in(&target_root)?;
        extracted_files.push(entry_path_string);
    }
    Ok(extracted_files)
}

fn validate_path_safety(entry_name: &str) -> Result<()> {
    let normalized_path = entry_name.replace('\\', "/");
    let path_components: Vec<&str> = normalized_path.split('/').filter(|s| !s.is_empty()).collect();

    let mut depth_counter: i32 = 0;
    for component in &path_components {
        if *component == ".." {
            depth_counter -= 1;
        } else {
            depth_counter += 1;
        }
        if depth_counter < 0 {
            return Err(CoreError::PathTraversal(entry_name.to_string()));
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    fn create_test_zip(entries: &[(&str, &[u8])]) -> Vec<u8> {
        let output_buffer = Vec::new();
        let mut zip_writer = ZipWriter::new(Cursor::new(output_buffer));
        let options = SimpleFileOptions::default().compression_method(zip::CompressionMethod::Deflated);

        for (name, data) in entries {
            if name.ends_with('/') {
                zip_writer.add_directory(*name, options).unwrap();
            } else {
                zip_writer.start_file(name, options).unwrap();
                zip_writer.write_all(data).unwrap();
            }
        }
        zip_writer.finish().unwrap().into_inner()
    }

    #[test]
    fn zip_list_and_read() {
        let zip_data = create_test_zip(&[("hello.txt", b"hello world"), ("dir/", b""), ("dir/nested.txt", b"nested content")]);
        let temp_dir = std::env::temp_dir().join("core_archive_test");
        fs::create_dir_all(&temp_dir).unwrap();
        let archive_path = temp_dir.join("test.zip");
        fs::write(&archive_path, &zip_data).unwrap();

        let entries = zip_list_entries(archive_path.to_str().unwrap()).unwrap();
        assert_eq!(entries.len(), 3);

        let content = zip_read_entry(archive_path.to_str().unwrap(), "hello.txt").unwrap();
        assert_eq!(content, b"hello world");

        let nested = zip_read_entry(archive_path.to_str().unwrap(), "dir/nested.txt").unwrap();
        assert_eq!(nested, b"nested content");

        assert!(zip_entry_exists(archive_path.to_str().unwrap(), "hello.txt").unwrap());
        assert!(!zip_entry_exists(archive_path.to_str().unwrap(), "missing.txt").unwrap());

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn zip_extract_single_file() {
        let zip_data = create_test_zip(&[("extract_me.txt", b"extract this")]);
        let temp_dir = std::env::temp_dir().join("core_archive_extract");
        fs::create_dir_all(&temp_dir).unwrap();
        let archive_path = temp_dir.join("test.zip");
        fs::write(&archive_path, &zip_data).unwrap();

        let target = temp_dir.join("output.txt");
        zip_extract_file(archive_path.to_str().unwrap(), "extract_me.txt", target.to_str().unwrap()).unwrap();

        let extracted = fs::read(&target).unwrap();
        assert_eq!(extracted, b"extract this");

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn zip_extract_directory() {
        let zip_data = create_test_zip(&[
            ("subdir/", b""),
            ("subdir/a.txt", b"aaa"),
            ("subdir/b.txt", b"bbb"),
            ("other.txt", b"other"),
        ]);
        let temp_dir = std::env::temp_dir().join("core_archive_extractdir");
        fs::create_dir_all(&temp_dir).unwrap();
        let archive_path = temp_dir.join("test.zip");
        fs::write(&archive_path, &zip_data).unwrap();

        let target_dir = temp_dir.join("extracted");
        let extracted = zip_extract_dir(archive_path.to_str().unwrap(), "subdir/", target_dir.to_str().unwrap()).unwrap();

        assert_eq!(extracted.len(), 2);
        let a_content = fs::read(target_dir.join("a.txt")).unwrap();
        assert_eq!(a_content, b"aaa");

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn zip_create_and_read_back() {
        let temp_dir = std::env::temp_dir().join("core_archive_create");
        fs::create_dir_all(&temp_dir).unwrap();
        let archive_path = temp_dir.join("created.zip");

        let entries = vec![
            ("file1.txt".to_string(), b"data1".to_vec()),
            ("file2.txt".to_string(), b"data2".to_vec()),
            ("nested/".to_string(), b"".to_vec()),
            ("nested/file3.txt".to_string(), b"data3".to_vec()),
        ];
        zip_create_from_entries(archive_path.to_str().unwrap(), &entries).unwrap();

        let read_back = zip_read_entry(archive_path.to_str().unwrap(), "file1.txt").unwrap();
        assert_eq!(read_back, b"data1");

        let read_back3 = zip_read_entry(archive_path.to_str().unwrap(), "nested/file3.txt").unwrap();
        assert_eq!(read_back3, b"data3");

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn zip_merge() {
        let temp_dir = std::env::temp_dir().join("core_archive_merge");
        fs::create_dir_all(&temp_dir).unwrap();

        let zip1 = create_test_zip(&[("a.txt", b"aaa"), ("shared.txt", b"from zip1")]);
        let zip2 = create_test_zip(&[("b.txt", b"bbb"), ("shared.txt", b"from zip2")]);

        let path1 = temp_dir.join("zip1.zip");
        let path2 = temp_dir.join("zip2.zip");
        fs::write(&path1, &zip1).unwrap();
        fs::write(&path2, &zip2).unwrap();

        let target = temp_dir.join("merged.zip");
        let mut exclude = HashSet::new();
        exclude.insert("shared.txt".to_string());

        zip_merge_archives(
            &[path1.to_str().unwrap(), path2.to_str().unwrap()],
            target.to_str().unwrap(),
            &exclude,
        )
        .unwrap();

        let content_a = zip_read_entry(target.to_str().unwrap(), "a.txt").unwrap();
        assert_eq!(content_a, b"aaa");
        let content_b = zip_read_entry(target.to_str().unwrap(), "b.txt").unwrap();
        assert_eq!(content_b, b"bbb");
        assert!(!zip_entry_exists(target.to_str().unwrap(), "shared.txt").unwrap());

        fs::remove_dir_all(&temp_dir).unwrap();
    }

    #[test]
    fn path_traversal_detection() {
        assert!(validate_path_safety("../../../etc/passwd").is_err());
        assert!(validate_path_safety("dir/../../escape.txt").is_err());
        assert!(validate_path_safety("normal/path/file.txt").is_ok());
        assert!(validate_path_safety("file.txt").is_ok());
        assert!(validate_path_safety("dir/subdir/file.txt").is_ok());
    }
}
