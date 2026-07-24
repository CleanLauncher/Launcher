use serde_json::Value;

use error::{CoreError, Result};

pub fn parse(input_bytes: &[u8]) -> Result<Value> {
    serde_json::from_slice(input_bytes).map_err(CoreError::Json)
}

pub fn parse_string(input_text: &str) -> Result<Value> {
    serde_json::from_str(input_text).map_err(CoreError::Json)
}

pub fn to_string(value: &Value) -> Result<String> {
    serde_json::to_string(value).map_err(CoreError::Json)
}

pub fn to_string_pretty(value: &Value) -> Result<String> {
    serde_json::to_string_pretty(value).map_err(CoreError::Json)
}

pub fn is_binary_json(input_bytes: &[u8]) -> bool {
    // Binary JSON format starts with 0x00 0xBF 0x00 0xBF
    input_bytes.len() >= 4 && input_bytes[0] == 0x00 && input_bytes[1] == 0xBF && input_bytes[2] == 0x00 && input_bytes[3] == 0xBF
}

pub fn require_document(input_bytes: &[u8], what: &str) -> Result<Value> {
    if is_binary_json(input_bytes) {
        return Err(CoreError::InvalidData(format!("{}: Invalid JSON. Binary JSON unsupported", what)));
    }
    parse(input_bytes)
}

pub fn require_object<'a>(value: &'a Value, what: &str) -> Result<&'a serde_json::Map<String, Value>> {
    value
        .as_object()
        .ok_or_else(|| CoreError::InvalidData(format!("{} is not an object", what)))
}

pub fn require_array<'a>(value: &'a Value, what: &str) -> Result<&'a Vec<Value>> {
    value
        .as_array()
        .ok_or_else(|| CoreError::InvalidData(format!("{} is not an array", what)))
}

pub fn require_string<'a>(value: &'a Value, what: &str) -> Result<&'a str> {
    value
        .as_str()
        .ok_or_else(|| CoreError::InvalidData(format!("{} is not a string", what)))
}

pub fn require_bool(value: &Value, what: &str) -> Result<bool> {
    value
        .as_bool()
        .ok_or_else(|| CoreError::InvalidData(format!("{} is not a bool", what)))
}

pub fn require_f64(value: &Value, what: &str) -> Result<f64> {
    value
        .as_f64()
        .ok_or_else(|| CoreError::InvalidData(format!("{} is not a number", what)))
}

pub fn require_i64(value: &Value, what: &str) -> Result<i64> {
    let num = require_f64(value, what)?;
    if num.fract() != 0.0 {
        return Err(CoreError::InvalidData(format!("{} is not an integer", what)));
    }
    Ok(num as i64)
}

pub fn require_uuid(value: &Value, what: &str) -> Result<String> {
    let s = require_string(value, what)?;
    // Validate UUID format
    uuid::Uuid::parse_str(s)
        .map(|_| s.to_string())
        .map_err(|_| CoreError::InvalidData(format!("{} is not a valid UUID", what)))
}

pub fn to_hex_string(input_bytes: &[u8]) -> String {
    input_bytes.iter().map(|b| format!("{:02x}", b)).collect()
}

pub fn from_hex_string(hex: &str) -> Result<Vec<u8>> {
    (0..hex.len())
        .step_by(2)
        .map(|i| {
            u8::from_str_radix(&hex[i..i + 2], 16).map_err(|_| CoreError::InvalidData(format!("Invalid hex string at position {}", i)))
        })
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parse_json() {
        let data = br#"{"key": "value"}"#;
        let value = parse(data).unwrap();
        assert_eq!(value["key"], "value");
    }

    #[test]
    fn is_binary_json_test() {
        assert!(is_binary_json(&[0x00, 0xBF, 0x00, 0xBF]));
        assert!(!is_binary_json(b"{\"key\": \"value\"}"));
    }

    #[test]
    fn hex_roundtrip() {
        let data = vec![0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF];
        let hex = to_hex_string(&data);
        assert_eq!(hex, "0123456789abcdef");
        let decoded = from_hex_string(&hex).unwrap();
        assert_eq!(data, decoded);
    }
}
