use digest::Digest;
use md5::Md5;
use sha2::{Sha256, Sha512};

use error::{CoreError, Result};

/// Computes SHA-256 hash of input bytes, returned as lowercase hex string.
pub fn sha256(input_bytes: &[u8]) -> String {
    let mut hasher = Sha256::new();
    hasher.update(input_bytes);
    hex_string(&hasher.finalize())
}

/// Computes SHA-512 hash of input bytes, returned as lowercase hex string.
pub fn sha512(input_bytes: &[u8]) -> String {
    let mut hasher = Sha512::new();
    hasher.update(input_bytes);
    hex_string(&hasher.finalize())
}

/// Computes MD5 hash of input bytes, returned as lowercase hex string.
pub fn md5(input_bytes: &[u8]) -> String {
    let mut hasher = Md5::new();
    hasher.update(input_bytes);
    hex_string(&hasher.finalize())
}

/// Verifies that input bytes match an expected SHA-256 hash.
pub fn verify_sha256(input_bytes: &[u8], expected_hash: &str) -> bool {
    sha256(input_bytes) == expected_hash.to_lowercase()
}

/// Verifies that input bytes match an expected SHA-512 hash.
pub fn verify_sha512(input_bytes: &[u8], expected_hash: &str) -> bool {
    sha512(input_bytes) == expected_hash.to_lowercase()
}

/// Verifies that input bytes match an expected MD5 hash.
pub fn verify_md5(input_bytes: &[u8], expected_hash: &str) -> bool {
    md5(input_bytes) == expected_hash.to_lowercase()
}

/// Hashes a file from disk using SHA-256.
pub fn sha256_file(path: &str) -> Result<String> {
    let contents = std::fs::read(path).map_err(CoreError::Io)?;
    Ok(sha256(&contents))
}

/// Hashes a file from disk using SHA-512.
pub fn sha512_file(path: &str) -> Result<String> {
    let contents = std::fs::read(path).map_err(CoreError::Io)?;
    Ok(sha512(&contents))
}

/// Hashes a file from disk using MD5.
pub fn md5_file(path: &str) -> Result<String> {
    let contents = std::fs::read(path).map_err(CoreError::Io)?;
    Ok(md5(&contents))
}

/// Computes SHA-256 hash incrementally from a reader.
pub fn sha256_reader<R: std::io::Read>(reader: &mut R) -> Result<String> {
    let mut hasher = Sha256::new();
    let mut buffer = [0u8; 8192];
    loop {
        let bytes_read = reader.read(&mut buffer).map_err(CoreError::Io)?;
        if bytes_read == 0 {
            break;
        }
        hasher.update(&buffer[..bytes_read]);
    }
    Ok(hex_string(&hasher.finalize()))
}

/// Computes SHA-512 hash incrementally from a reader.
pub fn sha512_reader<R: std::io::Read>(reader: &mut R) -> Result<String> {
    let mut hasher = Sha512::new();
    let mut buffer = [0u8; 8192];
    loop {
        let bytes_read = reader.read(&mut buffer).map_err(CoreError::Io)?;
        if bytes_read == 0 {
            break;
        }
        hasher.update(&buffer[..bytes_read]);
    }
    Ok(hex_string(&hasher.finalize()))
}

fn hex_string(bytes: &[u8]) -> String {
    bytes.iter().map(|b| format!("{:02x}", b)).collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn sha256_empty() {
        let result = sha256(b"");
        assert_eq!(result, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    }

    #[test]
    fn sha256_hello_world() {
        let result = sha256(b"hello world");
        assert_eq!(result, "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9");
    }

    #[test]
    fn sha512_hello_world() {
        let result = sha512(b"hello world");
        assert_eq!(
            result,
            "309ecc489c12d6eb4cc40f50c902f2b4d0ed77ee511a7c7a9bcd3ca86d4cd86f989dd35bc5ff499670da34255b45b0cfd830e81f605dcf7dc5542e93ae9cd76f"
        );
    }

    #[test]
    fn md5_hello_world() {
        let result = md5(b"hello world");
        assert_eq!(result, "5eb63bbbe01eeed093cb22bb8f5acdc3");
    }

    #[test]
    fn verify_sha256_correct() {
        assert!(verify_sha256(
            b"hello world",
            "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9"
        ));
    }

    #[test]
    fn verify_sha256_incorrect() {
        assert!(!verify_sha256(
            b"hello world",
            "0000000000000000000000000000000000000000000000000000000000000000"
        ));
    }

    #[test]
    fn verify_case_insensitive() {
        assert!(verify_sha256(
            b"hello world",
            "B94D27B9934D3E08A52E52D7DA7DABFAC484EFE37A5380EE9088F7ACE2EFCDE9"
        ));
    }

    #[test]
    fn sha256_file_nonexistent() {
        assert!(sha256_file("/nonexistent/file").is_err());
    }

    #[test]
    fn sha256_reader_empty() {
        let empty: &[u8] = b"";
        let mut cursor = std::io::Cursor::new(empty);
        let result = sha256_reader(&mut cursor).unwrap();
        assert_eq!(result, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    }

    #[test]
    fn sha256_reader_hello() {
        let data = b"hello world";
        let mut cursor = std::io::Cursor::new(data);
        let result = sha256_reader(&mut cursor).unwrap();
        assert_eq!(result, "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9");
    }
}
