<?php

// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

function IsBlock()
{
	global $g_locals;
	return !empty ( $g_locals['block_mode'] );
}


// The shared default covers both indexer tables and CREATE TABLE over SQL/HTTP.
function BlockPostingsConfig ( $config )
{
	return preg_replace_callback ( '/(^[\t ]*searchd\s*\{)(.*?)(^[\t ]*\})/ms', function ( $match )
	{
		if ( preg_match ( '/^[\t ]*postings_format\s*=/m', $match[2] ) )
			return $match[0];

		return $match[1]."\n\tpostings_format = block\n".$match[2].$match[3];
	}, $config );
}
