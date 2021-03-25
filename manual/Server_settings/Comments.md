# Comments

The configuration file supports comments, with `#` character used as start comment section. The comment character can be present at the start of the line or inline.

Extra care should be considered when using `#` in character tokenization settings as everything after it will not be taken into consideration. To avoid this, use `#` UTF-8 which is U+23.

`#` can also be escaped using `\`. Escaping is required if `#` is present in database credential in source declarations.

