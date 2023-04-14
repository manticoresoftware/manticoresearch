# Comments

Manticore Search's configuration file supports comments, which help provide explanations or notes within the configuration file. The `#` character is used to start a comment section. You can place the comment character either at the beginning of a line or inline within a line.

When using comments, be cautious when incorporating the `#` character in character tokenization settings, as everything following it will be ignored. To prevent this, use the `#` UTF-8 code, which is U+23.

If you need to use the `#` character within your configuration file, such as within database credentials in source declarations, you can escape it using a backslash `\`. This allows you to include the `#` character in your settings without it being interpreted as the start of a comment.

<!-- proofread -->