#!/bin/bash

# indexer

srcFile="../manual/Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md"
destFile="indexer.1"

cp $srcFile tmp.md

# Remove all HTML comments
sed -i '' '/<!--/,/-->/d' tmp.md

# Remove all markdown links
sed -i '' 's/\[\([^\]]*\)\]([^)]*)/\1/g' tmp.md

# Remove all before specified string, but not the string itself, and change the string to the new one
sed -i '' -n '/## Indexer tool/,$p' tmp.md
sed -i '' '1s/## Indexer tool/## DESCRIPTION/' tmp.md

# Replace the long section name to just OPTIONS as the latter is canonical
sed -i '' 's/### Indexer command line arguments/## OPTIONS/g' tmp.md

# Remove all after specified string, including the string
sed -i '' '/### Indexer configuration settings/,$d' tmp.md

# Get OS type
osType=$(uname)

# Get mtime of the original file in the manual from where we generate the man page
if [[ "$osType" == "Linux" ]]; then
    mtime=$(stat -c %y "$srcFile" | awk '{print $1}' | awk -F '-' '{print $3"/"$2"/"$1}')
elif [[ "$osType" == "Darwin" ]]; then  # For macOS
    mtime=$(stat -f "%Sm" -t "%d/%m/%Y" "$srcFile")
else
    echo "Unsupported OS"
    exit 1
fi

# Prepare a header of the indexer man page
cat << EOF > $destFile
'\" t
.\"     Title: indexer
.\"    Author: [see the "Author" section]
.\"    Manual: Manticore Search
.\"    Source: Manticore Search
.\"  Language: English
.\"
.TH "INDEXER" "1" "$mtime" "Manticore Search" ""
.SH "NAME"
indexer \- Manticore Search Indexer Tool
.SH "SYNOPSIS"
sudo -u manticore indexer ...
EOF

# Convert the temporary markdown file to man page
md2man-roff tmp.md >> $destFile

# Add a footer of the man page
cat << EOF >> $destFile
.SH "AUTHOR"
.PP
Manticore Software LTD (https://manticoresearch\&.com)
.SH "COPYRIGHT"
.PP
Copyright 2017\-2023 Manticore Software LTD (https://manticoresearch\&.com), 2008\-2016 Sphinx Technologies Inc (http://sphinxsearch\&.com), 2001\-2016 Andrew Aksyonoff
.PP
Permission is granted to copy, distribute and/or modify this document under the terms of the GNU General Public License, Version 2 or any later version published by the Free Software Foundation\&.
.SH "SEE ALSO"
.PP
\fBsearchd\fR(1),
\fBsearch\fR(1),
\fBindextool\fR(1),
\fBspelldump\fR(1)
.PP
Manticore Search and its related programs are thoroughly documented
in the \fIManticore Search reference manual\fR, which is accessible
at https://manual.manticoresearch.com/
EOF

rm  tmp.*
