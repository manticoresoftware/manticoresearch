<?php

$text = file_get_contents('../../manual/Introduction.md');

// Remove all HTML comments
$text = preg_replace('/<!--.*?-->/is', '', $text);

// Remove all markdown links
$text = preg_replace('/\[(.*?)\]\(.*?\)/', '$1', $text);

// Replace the title
$text = str_replace("# Introduction", "# Manticore", $text);

file_put_contents('README.Debian.in', $text);
