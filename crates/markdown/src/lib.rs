use pulldown_cmark::{html, Options, Parser};

use error::Result;

pub fn markdown_to_html(markdown: &str) -> Result<String> {
    let mut render_options = Options::empty();
    render_options.insert(Options::ENABLE_TABLES);
    render_options.insert(Options::ENABLE_FOOTNOTES);
    render_options.insert(Options::ENABLE_STRIKETHROUGH);
    render_options.insert(Options::ENABLE_TASKLISTS);

    let parser = Parser::new_ext(markdown, render_options);
    let mut html_output = String::new();
    html::push_html(&mut html_output, parser);

    Ok(html_output)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn basic_markdown() {
        let result = markdown_to_html("**bold**").unwrap();
        assert!(result.contains("<strong>bold</strong>"));
    }

    #[test]
    fn empty_input() {
        let result = markdown_to_html("").unwrap();
        assert!(result.is_empty() || result.trim().is_empty());
    }
}
