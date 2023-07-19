#!/bin/bash

# Get OS type to calculate mtime properly later
osType=$(uname)
authorAndCopyright=$(cat <<-END
.SH "AUTHOR"
.PP
Manticore Software LTD (https://manticoresearch\&.com)
.SH "COPYRIGHT"
.PP
Copyright 2017\-2023 Manticore Software LTD (https://manticoresearch\&.com), 2008\-2016 Sphinx Technologies Inc (http://sphinxsearch\&.com), 2001\-2016 Andrew Aksyonoff
.PP
Permission is granted to copy, distribute and/or modify this document under the terms of the GNU General Public License, Version 2 or any later version published by the Free Software Foundation\&.
END

)

# indexer

srcFile="../manual/Data_creation_and_modification/Adding_data_from_external_storages/Plain_tables_creation.md"
destFile="indexer.1"

cp $srcFile tmp.md

# Remove all HTML comments
sed -i '' '/<!--/,/-->/d' tmp.md

# Remove all markdown links
sed -i '' -r 's/\[([^]]+)\]\([^)]+\)/\`\1\`/g' tmp.md

# Remove all before specified string, but not the string itself, and change the string to the new one
sed -i '' -n '/## Indexer tool/,$p' tmp.md
sed -i '' '1s/## Indexer tool/## DESCRIPTION/' tmp.md

# Replace the long section name to just OPTIONS as the latter is canonical
sed -i '' 's/### Indexer command line arguments/## OPTIONS/g' tmp.md

# Remove all after specified string, including the string
sed -i '' '/### Indexer configuration settings/,$d' tmp.md

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
sudo \-u manticore indexer ...
EOF

# Convert the temporary markdown file to man page
md2man-roff tmp.md >> $destFile

# Add a footer of the man page
cat << EOF >> $destFile
$authorAndCopyright
.SH "SEE ALSO"
.PP
\fBsearchd\fR(1),
\fBindextool\fR(1)
.PP
Manticore Search and its related programs are thoroughly documented
in the \fIManticore Search reference manual\fR, which is accessible
at https://manual.manticoresearch.com/
EOF

rm tmp.*

# searchd

srcFile="../manual/Starting_the_server/Manually.md"
destFile="searchd.1"

cp $srcFile tmp.md

# Remove all HTML comments
sed -i '' '/<!--/,/-->/d' tmp.md

# Remove all markdown links
sed -i '' -r 's/\[([^]]+)\]\([^)]+\)/\`\1\`/g' tmp.md

# Remove all before specified string, but not the string itself, and change the string to the new one
sed -i '' -n '/## searchd command line options/,$p' tmp.md
sed -i '' '1s/## searchd command line options/## OPTIONS/' tmp.md

# Capitalize section names
sed -i '' 's/## Environment variables/## ENVIRONMENT VARIABLES/g' tmp.md
sed -i '' 's/## Signals/## SIGNALS/g' tmp.md
sed -i '' 's/## Plugin dir/## PLUGIN DIR/g' tmp.md

# Prepare a header of the indexer man page
cat << EOF > $destFile
'\" t
.\"     Title: searchd
.\"    Author: [see the "Author" section]
.\"    Manual: Manticore Search
.\"    Source: Manticore Search
.\"  Language: English
.\"
.TH "SEARCHD" "1" "$mtime" "Manticore Search" ""
.SH "NAME"
searchd \- Manticore Search Server
.SH "SYNOPSIS"
.HP \w'\fBsearchd\fR\ 'u
\fBsearchd\fR [\-\-config\ \fICONFIGFILE\fR] [OPTION ...]
.HP \w'\fBsearchd\fR\ 'u
\fBsearchd\fR \-\-stopwait [\-\-config\ \fICONFIGFILE\fR]
EOF

# Convert the temporary markdown file to man page
md2man-roff tmp.md >> $destFile

# Add a footer of the man page
cat << EOF >> $destFile
$authorAndCopyright
.SH "SEE ALSO"
.PP
\fBindexer\fR(1),
\fBindextool\fR(1)
.PP
Manticore Search and its related programs are thoroughly documented
in the \fIManticore Search reference manual\fR, which is accessible
at https://manual.manticoresearch.com/
EOF

rm tmp.*

srcFile="../manual/Miscellaneous_tools.md"
destFile="indextool.1"

cp $srcFile tmp.md

# Remove all HTML comments
sed -i '' '/<!--/,/-->/d' tmp.md

# Remove all markdown links
sed -i '' -r 's/\[([^]]+)\]\([^)]+\)/\`\1\`/g' tmp.md

# Remove all before specified string, but not the string itself, and change the string to the new one
sed -i '' -n '/## indextool/,$p' tmp.md
sed -i '' '1s/## indextool/## DESCRIPTION/' tmp.md

# Replace the long section name to just OPTIONS as the latter is canonical
sed -i '' 's/### Options/## OPTIONS/g' tmp.md

# Remove all after specified string, including the string
sed -i '' '/## spelldump/,$d' tmp.md

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
.TH "INDEXTOOL" "1" "$mtime" "Manticore Search" ""
.SH "NAME"
indextool
.SH "SYNOPSIS"
indextool <command> [options]
EOF

# Convert the temporary markdown file to man page
md2man-roff tmp.md >> $destFile

# Add a footer of the man page
cat << EOF >> $destFile
$authorAndCopyright
.SH "SEE ALSO"
.PP
\fBsearchd\fR(1),
\fBindexer\fR(1)
.PP
Manticore Search and its related programs are thoroughly documented
in the \fIManticore Search reference manual\fR, which is accessible
at https://manual.manticoresearch.com/
EOF

rm tmp.*
