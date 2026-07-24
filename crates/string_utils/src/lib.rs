use regex::Regex;

pub fn natural_compare(left_string: &str, right_string: &str, case_insensitive: bool) -> std::cmp::Ordering {
    let mut left_chars = left_string.chars().peekable();
    let mut right_chars = right_string.chars().peekable();

    loop {
        // Skip whitespace on both sides.
        while left_chars.peek().is_some_and(|c| c.is_whitespace()) {
            left_chars.next();
        }
        while right_chars.peek().is_some_and(|c| c.is_whitespace()) {
            right_chars.next();
        }

        let left_char = left_chars.next();
        let right_char = right_chars.next();

        match (left_char, right_char) {
            (None, None) => return std::cmp::Ordering::Equal,
            (None, _) => return std::cmp::Ordering::Less,
            (_, None) => return std::cmp::Ordering::Greater,
            (Some(left_char), Some(right_char)) => {
                let left_char = if case_insensitive {
                    left_char.to_lowercase().next().unwrap_or(left_char)
                } else {
                    left_char
                };
                let right_char = if case_insensitive {
                    right_char.to_lowercase().next().unwrap_or(right_char)
                } else {
                    right_char
                };

                if left_char.is_ascii_digit() && right_char.is_ascii_digit() {
                    // Collect the full digit runs so we can compare by length then lexically.
                    let mut left_digits = String::new();
                    let mut right_digits = String::new();

                    if left_char == '0' {
                        left_digits.push(left_char);
                        while left_chars.peek().is_some_and(|c| *c == '0') {
                            left_digits.push(left_chars.next().unwrap());
                        }
                    }
                    if left_chars.peek().is_some_and(|c| c.is_ascii_digit()) {
                        while left_chars.peek().is_some_and(|c| c.is_ascii_digit()) {
                            left_digits.push(left_chars.next().unwrap());
                        }
                    }

                    if right_char == '0' {
                        right_digits.push(right_char);
                        while right_chars.peek().is_some_and(|c| *c == '0') {
                            right_digits.push(right_chars.next().unwrap());
                        }
                    }
                    if right_chars.peek().is_some_and(|c| c.is_ascii_digit()) {
                        while right_chars.peek().is_some_and(|c| c.is_ascii_digit()) {
                            right_digits.push(right_chars.next().unwrap());
                        }
                    }

                    // A longer digit run is the larger number.
                    if left_digits.len() != right_digits.len() {
                        return left_digits.len().cmp(&right_digits.len());
                    }
                    let digit_ordering = left_digits.cmp(&right_digits);
                    if digit_ordering != std::cmp::Ordering::Equal {
                        return digit_ordering;
                    }
                } else {
                    let char_ordering = left_char.cmp(&right_char);
                    if char_ordering != std::cmp::Ordering::Equal {
                        return char_ordering;
                    }
                }
            }
        }
    }
}

pub fn human_readable_file_size(raw_byte_count: f64, use_si_units: bool, decimal_points: usize) -> String {
    let kibibyte_units = ["B", "KiB", "MiB", "GiB", "TiB"];
    let si_units = ["B", "KB", "MB", "GB", "TB"];
    let selected_units = if use_si_units { &si_units } else { &kibibyte_units };
    let unit_scale = if use_si_units { 1000.0 } else { 1024.0 };

    let mut scaled_value = raw_byte_count;
    let mut unit_index = 0;
    let rounding_factor = 10f64.powi(decimal_points as i32);

    while unit_index < selected_units.len() - 1 && (scaled_value * rounding_factor).round() / rounding_factor >= unit_scale {
        scaled_value /= unit_scale;
        unit_index += 1;
    }

    format!("{:.width$} {}", scaled_value, selected_units[unit_index], width = decimal_points)
}

pub fn truncate_url_human_friendly(url_input: &str, max_length: usize, _hard_limit: bool) -> String {
    if url_input.len() <= max_length {
        return url_input.to_string();
    }

    let truncated_url = &url_input[..max_length.saturating_sub(3)];
    format!("{}...", truncated_url)
}

pub fn get_random_alpha_numeric() -> String {
    uuid::Uuid::new_v4().to_string().replace('-', "")
}

pub fn split_first(input_string: &str, separator: &str, case_insensitive: bool) -> (String, String) {
    let match_index = if case_insensitive {
        input_string.to_lowercase().find(&separator.to_lowercase())
    } else {
        input_string.find(separator)
    };

    match match_index {
        Some(found_index) => {
            let before_separator = input_string[..found_index].to_string();
            let after_separator = input_string[found_index + separator.len()..].to_string();
            (before_separator, after_separator)
        }
        None => (input_string.to_string(), String::new()),
    }
}

pub fn split_first_char(input_string: &str, separator: char, case_insensitive: bool) -> (String, String) {
    let match_index = if case_insensitive {
        input_string
            .to_lowercase()
            .chars()
            .position(|c| c == separator.to_lowercase().next().unwrap_or(separator))
    } else {
        input_string.chars().position(|c| c == separator)
    };

    match match_index {
        Some(found_index) => {
            let before_separator = input_string[..found_index].to_string();
            let after_separator = input_string[found_index + 1..].to_string();
            (before_separator, after_separator)
        }
        None => (input_string.to_string(), String::new()),
    }
}

pub fn html_list_patch(html_input: &str) -> String {
    let list_close_regex = Regex::new(r"</\s*ul\s*>").unwrap();
    let mut patched_html = html_input.to_string();
    let mut scan_position = 0;

    while let Some(regex_match) = list_close_regex.find(&patched_html[scan_position..]) {
        let match_start = scan_position + regex_match.start();
        let after_list_close = match_start + regex_match.len();

        // Locate the end of the matched tag.
        if let Some(tag_end) = patched_html[after_list_close..].find('>') {
            let after_tag = after_list_close + tag_end + 1;

            // Look for an <img> sitting between this tag and the next list close.
            if let Some(next_close) = patched_html[after_tag..].find("</") {
                let between_text = &patched_html[after_tag..after_tag + next_close];
                if between_text.trim().is_empty() && between_text.contains("<img ") {
                    patched_html.insert_str(after_tag, "<br>");
                    scan_position = after_tag + 4;
                } else {
                    scan_position = after_tag;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }

    patched_html
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn natural_compare_test() {
        assert_eq!(natural_compare("file2", "file10", false), std::cmp::Ordering::Less);
        assert_eq!(natural_compare("file10", "file2", false), std::cmp::Ordering::Greater);
    }

    #[test]
    fn human_readable_file_size_test() {
        assert_eq!(human_readable_file_size(1024.0, false, 1), "1.0 KiB");
        assert_eq!(human_readable_file_size(1000.0, true, 1), "1.0 KB");
        assert_eq!(human_readable_file_size(500.0, false, 1), "500.0 B");
        assert_eq!(human_readable_file_size(0.0, false, 1), "0.0 B");
    }

    #[test]
    fn split_first_test() {
        assert_eq!(split_first("key=value", "=", false), ("key".to_string(), "value".to_string()));
        assert_eq!(split_first("no-separator", "=", false), ("no-separator".to_string(), String::new()));
    }
}
