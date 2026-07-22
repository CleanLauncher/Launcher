use thiserror::Error;

#[derive(Error, Debug)]
pub enum CoreError {
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),

    #[error("Compression error: {0}")]
    Compression(String),

    #[error("Zip error: {0}")]
    Zip(String),

    #[error("Tar error: {0}")]
    Tar(String),

    #[error("UTF-8 error: {0}")]
    Utf8(#[from] std::str::Utf8Error),

    #[error("JSON error: {0}")]
    Json(#[from] serde_json::Error),

    #[error("TOML error: {0}")]
    Toml(#[from] toml::de::Error),

    #[error("Markdown error: {0}")]
    Markdown(String),

    #[error("Path traversal detected: {0}")]
    PathTraversal(String),

    #[error("Invalid data: {0}")]
    InvalidData(String),

    #[error("Configuration error: {0}")]
    Config(String),
}

pub type Result<T> = std::result::Result<T, CoreError>;

impl From<zip::result::ZipError> for CoreError {
    fn from(err: zip::result::ZipError) -> Self {
        CoreError::Zip(err.to_string())
    }
}

impl From<flate2::DecompressError> for CoreError {
    fn from(err: flate2::DecompressError) -> Self {
        CoreError::Compression(err.to_string())
    }
}

impl From<flate2::CompressError> for CoreError {
    fn from(err: flate2::CompressError) -> Self {
        CoreError::Compression(err.to_string())
    }
}

impl From<tar::Error> for CoreError {
    fn from(err: tar::Error) -> Self {
        CoreError::Tar(err.to_string())
    }
}
