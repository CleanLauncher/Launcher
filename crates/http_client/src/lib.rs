use std::collections::HashMap;
use std::fs::{self, OpenOptions};
use std::io::{Read as _, Write as _};
use std::path::Path;
use std::sync::Mutex;
use std::time::Duration;

use error::{CoreError, Result};
use once_cell::sync::Lazy;
use reqwest::blocking::Client;
use reqwest::header::{HeaderMap, HeaderName, HeaderValue, USER_AGENT};

const DEFAULT_TIMEOUT_MS: u64 = 30_000;
const DEFAULT_MAX_RETRIES: u32 = 3;
const RETRY_BASE_DELAY_MS: u64 = 1_000;

struct HttpClientState {
    user_agent: String,
    timeout: Duration,
    extra_headers: HashMap<String, String>,
}

static STATE: Lazy<Mutex<HttpClientState>> = Lazy::new(|| {
    Mutex::new(HttpClientState {
        user_agent: String::from("Launcher/1.0"),
        timeout: Duration::from_millis(DEFAULT_TIMEOUT_MS),
        extra_headers: HashMap::new(),
    })
});

fn build_client() -> Result<Client> {
    let state = STATE.lock().map_err(|e| CoreError::Http(e.to_string()))?;
    let mut headers = HeaderMap::new();
    if let Ok(val) = HeaderValue::from_str(&state.user_agent) {
        headers.insert(USER_AGENT, val);
    }
    for (key, value) in &state.extra_headers {
        if let (Ok(name), Ok(val)) = (HeaderName::from_bytes(key.as_bytes()), HeaderValue::from_str(value)) {
            headers.insert(name, val);
        }
    }
    let timeout = state.timeout;
    drop(state);
    Client::builder()
        .timeout(timeout)
        .danger_accept_invalid_certs(false)
        .default_headers(headers)
        .build()
        .map_err(|e| CoreError::Http(e.to_string()))
}

pub fn set_user_agent(agent: &str) {
    if let Ok(mut state) = STATE.lock() {
        state.user_agent = agent.to_string();
    }
}

pub fn set_timeout(timeout_ms: u64) {
    if let Ok(mut state) = STATE.lock() {
        state.timeout = Duration::from_millis(timeout_ms);
    }
}

pub fn set_header(name: &str, value: &str) {
    if let Ok(mut state) = STATE.lock() {
        state.extra_headers.insert(name.to_string(), value.to_string());
    }
}

pub struct HttpResponse {
    pub status: u16,
    pub body: Vec<u8>,
}

pub struct HttpDownloadResult {
    pub status: u16,
    pub bytes_written: u64,
}

pub fn get(url: &str) -> Result<HttpResponse> {
    request_with_retry(url, None, None, None)
}

pub fn get_with_headers(url: &str, headers: &[(String, String)]) -> Result<HttpResponse> {
    let header_map: HashMap<String, String> = headers.iter().cloned().collect();
    request_with_retry(url, None, Some(header_map), None)
}

pub fn download_to_file(url: &str, path: &str) -> Result<HttpDownloadResult> {
    download_to_file_with_resume(url, path, 0, None)
}

pub fn download_to_file_with_resume(url: &str, path: &str, existing_bytes: u64, max_retries: Option<u32>) -> Result<HttpDownloadResult> {
    let max_retries = max_retries.unwrap_or(DEFAULT_MAX_RETRIES);
    let mut last_error: Option<CoreError> = None;
    let mut extra_headers = HashMap::new();
    if existing_bytes > 0 {
        extra_headers.insert("Range".to_string(), format!("bytes={}-", existing_bytes));
    }
    let mut attempts = 0;
    while attempts <= max_retries {
        match execute_download(url, path, existing_bytes, &extra_headers) {
            Ok(result) => return Ok(result),
            Err(e) => {
                if is_client_error(&e) {
                    return Err(e);
                }
                last_error = Some(e);
                attempts += 1;
                if attempts <= max_retries {
                    let delay = calculate_retry_delay(attempts, &last_error);
                    std::thread::sleep(Duration::from_millis(delay));
                }
            }
        }
    }
    Err(last_error.unwrap_or_else(|| CoreError::Http("max retries exceeded".to_string())))
}

fn execute_download(url: &str, path: &str, existing_bytes: u64, extra_headers: &HashMap<String, String>) -> Result<HttpDownloadResult> {
    let client = build_client()?;
    let mut request = client.get(url);
    for (key, value) in extra_headers {
        if let (Ok(name), Ok(val)) = (HeaderName::from_bytes(key.as_bytes()), HeaderValue::from_str(value)) {
            request = request.header(name, val);
        }
    }
    let response = request.send().map_err(|e| CoreError::Http(e.to_string()))?;
    let status = response.status().as_u16();
    if status == 200 || status == 206 {
        let is_partial = status == 206;
        let open_result = if existing_bytes > 0 && is_partial {
            OpenOptions::new().append(true).open(path)
        } else if existing_bytes > 0 && status == 200 {
            OpenOptions::new().write(true).truncate(true).open(path)
        } else {
            fs::create_dir_all(Path::new(path).parent().unwrap_or(Path::new("."))).map_err(CoreError::Io)?;
            OpenOptions::new().create(true).write(true).truncate(true).open(path)
        };
        let mut file = open_result.map_err(CoreError::Io)?;
        let mut bytes_written = if status == 206 { existing_bytes } else { 0 };
        let mut buffer = [0u8; 8192];
        let mut reader = response;
        loop {
            let bytes_read = reader.read(&mut buffer).map_err(CoreError::Io)?;
            if bytes_read == 0 {
                break;
            }
            file.write_all(&buffer[..bytes_read]).map_err(CoreError::Io)?;
            bytes_written += bytes_read as u64;
        }
        file.flush().map_err(CoreError::Io)?;
        Ok(HttpDownloadResult { status, bytes_written })
    } else {
        Err(CoreError::Http(format!("HTTP {}", status)))
    }
}

fn request_with_retry(
    url: &str,
    method: Option<&str>,
    extra_headers: Option<HashMap<String, String>>,
    max_retries: Option<u32>,
) -> Result<HttpResponse> {
    let max_retries = max_retries.unwrap_or(DEFAULT_MAX_RETRIES);
    let mut last_error: Option<CoreError> = None;
    let mut attempts = 0;
    while attempts <= max_retries {
        match execute_request(url, method, extra_headers.as_ref()) {
            Ok(response) => return Ok(response),
            Err(e) => {
                if is_client_error(&e) {
                    return Err(e);
                }
                last_error = Some(e);
                attempts += 1;
                if attempts <= max_retries {
                    let delay = calculate_retry_delay(attempts, &last_error);
                    std::thread::sleep(Duration::from_millis(delay));
                }
            }
        }
    }
    Err(last_error.unwrap_or_else(|| CoreError::Http("max retries exceeded".to_string())))
}

fn execute_request(url: &str, method: Option<&str>, extra_headers: Option<&HashMap<String, String>>) -> Result<HttpResponse> {
    let client = build_client()?;
    let mut request = match method {
        Some("POST") | Some("post") => client.post(url),
        Some("PUT") | Some("put") => client.put(url),
        Some("DELETE") | Some("delete") => client.delete(url),
        _ => client.get(url),
    };
    if let Some(headers) = extra_headers {
        for (key, value) in headers {
            if let (Ok(name), Ok(val)) = (HeaderName::from_bytes(key.as_bytes()), HeaderValue::from_str(value)) {
                request = request.header(name, val);
            }
        }
    }
    let response = request.send().map_err(|e| CoreError::Http(e.to_string()))?;
    let status = response.status().as_u16();
    let body = response.bytes().map_err(|e| CoreError::Http(e.to_string()))?.to_vec();
    Ok(HttpResponse { status, body })
}

pub fn post_json(url: &str, body: &[u8], extra_headers: Option<HashMap<String, String>>) -> Result<HttpResponse> {
    let client = build_client()?;
    let mut request = client.post(url).header("Content-Type", "application/json").body(body.to_vec());
    if let Some(headers) = extra_headers {
        for (key, value) in &headers {
            if let (Ok(name), Ok(val)) = (HeaderName::from_bytes(key.as_bytes()), HeaderValue::from_str(value)) {
                request = request.header(name, val);
            }
        }
    }
    let response = request.send().map_err(|e| CoreError::Http(e.to_string()))?;
    let status = response.status().as_u16();
    let resp_body = response.bytes().map_err(|e| CoreError::Http(e.to_string()))?.to_vec();
    Ok(HttpResponse { status, body: resp_body })
}

fn is_client_error(error: &CoreError) -> bool {
    if let CoreError::Http(msg) = error {
        if let Some(code_str) = msg.strip_prefix("HTTP ") {
            if let Ok(code) = code_str.parse::<u16>() {
                return (400..500).contains(&code) && code != 429;
            }
        }
    }
    false
}

fn calculate_retry_delay(attempt: u32, last_error: &Option<CoreError>) -> u64 {
    if let Some(CoreError::Http(msg)) = last_error {
        if msg.contains("429") {
            return RETRY_BASE_DELAY_MS * 2u64.pow(attempt);
        }
    }
    RETRY_BASE_DELAY_MS * 2u64.pow(attempt - 1)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_client_error_4xx() {
        assert!(is_client_error(&CoreError::Http("HTTP 400".to_string())));
        assert!(is_client_error(&CoreError::Http("HTTP 404".to_string())));
        assert!(is_client_error(&CoreError::Http("HTTP 422".to_string())));
    }

    #[test]
    fn test_is_client_error_429_not_client() {
        assert!(!is_client_error(&CoreError::Http("HTTP 429".to_string())));
    }

    #[test]
    fn test_is_client_error_5xx() {
        assert!(!is_client_error(&CoreError::Http("HTTP 500".to_string())));
        assert!(!is_client_error(&CoreError::Http("HTTP 503".to_string())));
    }

    #[test]
    fn test_is_client_error_non_http() {
        assert!(!is_client_error(&CoreError::Io(std::io::Error::new(
            std::io::ErrorKind::Other,
            "test"
        ))));
    }

    #[test]
    fn test_calculate_retry_delay_normal() {
        let delay = calculate_retry_delay(1, &None);
        assert_eq!(delay, 1000);
        let delay = calculate_retry_delay(2, &None);
        assert_eq!(delay, 2000);
        let delay = calculate_retry_delay(3, &None);
        assert_eq!(delay, 4000);
    }

    #[test]
    fn test_calculate_retry_delay_rate_limit() {
        let err = Some(CoreError::Http("HTTP 429".to_string()));
        let delay = calculate_retry_delay(1, &err);
        assert_eq!(delay, 2000);
        let delay = calculate_retry_delay(2, &err);
        assert_eq!(delay, 4000);
    }

    #[test]
    fn test_set_user_agent() {
        set_user_agent("TestAgent/2.0");
        let state = STATE.lock().unwrap();
        assert_eq!(state.user_agent, "TestAgent/2.0");
    }

    #[test]
    fn test_set_timeout() {
        set_timeout(5000);
        let state = STATE.lock().unwrap();
        assert_eq!(state.timeout, Duration::from_millis(5000));
    }

    #[test]
    fn test_set_header() {
        set_header("X-Custom", "value123");
        let state = STATE.lock().unwrap();
        assert_eq!(state.extra_headers.get("X-Custom").unwrap(), "value123");
    }
}
