use flate2::read::{DeflateDecoder, GzDecoder};
use flate2::write::{DeflateEncoder, GzEncoder};
use flate2::Compression;
use std::io::{Read, Write};

use crate::error::{CoreError, Result};

pub fn unzip(compressed_input: &[u8]) -> Result<Vec<u8>> {
    if compressed_input.is_empty() {
        return Ok(Vec::new());
    }

    let mut decoder = GzDecoder::new(compressed_input);
    let mut decompressed_output = Vec::new();
    decoder
        .read_to_end(&mut decompressed_output)
        .map_err(|io_error| CoreError::Compression(io_error.to_string()))?;
    Ok(decompressed_output)
}

pub fn zip(raw_input: &[u8]) -> Result<Vec<u8>> {
    if raw_input.is_empty() {
        return Ok(Vec::new());
    }

    let mut encoder = GzEncoder::new(Vec::new(), Compression::default());
    encoder
        .write_all(raw_input)
        .map_err(|io_error| CoreError::Compression(io_error.to_string()))?;
    encoder
        .finish()
        .map_err(|io_error| CoreError::Compression(io_error.to_string()))
}

pub fn inflate_raw(compressed_input: &[u8]) -> Result<Vec<u8>> {
    if compressed_input.is_empty() {
        return Ok(Vec::new());
    }

    let mut decoder = DeflateDecoder::new(compressed_input);
    let mut decompressed_output = Vec::new();
    decoder
        .read_to_end(&mut decompressed_output)
        .map_err(|io_error| CoreError::Compression(io_error.to_string()))?;
    Ok(decompressed_output)
}

pub fn deflate_raw(raw_input: &[u8]) -> Result<Vec<u8>> {
    if raw_input.is_empty() {
        return Ok(Vec::new());
    }

    let mut encoder = DeflateEncoder::new(Vec::new(), Compression::default());
    encoder
        .write_all(raw_input)
        .map_err(|io_error| CoreError::Compression(io_error.to_string()))?;
    encoder
        .finish()
        .map_err(|io_error| CoreError::Compression(io_error.to_string()))
}

pub fn read_gz_by_blocks<Reader, BlockHandler>(
    reader: &mut Reader,
    mut block_handler: BlockHandler,
) -> Result<()>
where
    Reader: Read,
    BlockHandler: FnMut(&[u8]) -> bool,
{
    let mut decoder = GzDecoder::new(reader);
    let mut read_buffer = [0u8; 16384];
    loop {
        let bytes_read = decoder
            .read(&mut read_buffer)
            .map_err(|io_error| CoreError::Compression(io_error.to_string()))?;
        if bytes_read == 0 {
            break;
        }
        if !block_handler(&read_buffer[..bytes_read]) {
            break;
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn roundtrip_gzip() {
        let sample_payload = b"Hello, World! This is a test of gzip compression.";
        let compressed_payload = zip(sample_payload).unwrap();
        let decompressed_payload = unzip(&compressed_payload).unwrap();
        assert_eq!(sample_payload.as_slice(), decompressed_payload.as_slice());
    }

    #[test]
    fn roundtrip_deflate() {
        let sample_payload = b"Hello, World! This is a test of deflate compression.";
        let compressed_payload = deflate_raw(sample_payload).unwrap();
        let decompressed_payload = inflate_raw(&compressed_payload).unwrap();
        assert_eq!(sample_payload.as_slice(), decompressed_payload.as_slice());
    }

    #[test]
    fn empty_input() {
        assert!(zip(b"").unwrap().is_empty());
        assert!(unzip(b"").unwrap().is_empty());
    }
}
