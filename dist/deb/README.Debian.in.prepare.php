<?php

$text = file_get_contents('../../manual/english/Introduction.md');

// Remove all HTML comments
$text = preg_replace('/<!--.*?-->/is', '', $text);

// Remove all markdown links
$text = preg_replace('/\[(.*?)\]\(.*?\)/', '$1', $text);

// Replace the title
$text = str_replace("# Introduction", "# Manticore", $text);

$text .= "## System Limits
Manticore Search requires increased memory mapping limits for large datasets. The installation sets vm.max_map_count=262144 in /etc/sysctl.d/70-manticore.conf. Verify with 'sysctl vm.max_map_count'.
";

file_put_contents('README.Debian.in', $text);
