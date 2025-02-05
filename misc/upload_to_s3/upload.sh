#!/bin/sh
set -e

# S3 credentials
S3_HOST="http://s3.manticoresearch.com:9000"
S3_ACCESS_KEY="manticore"
S3_SECRET_KEY="manticore"
S3_BUCKET="write-only"

# Ensure an upload directory is provided
if [ -z "$(ls -A /upload 2>/dev/null)" ]; then
  echo "Error: The /upload directory is empty. Please mount a directory with files to upload."
  exit 1
fi

# Initialize Minio client
mc alias set manticore "$S3_HOST" "$S3_ACCESS_KEY" "$S3_SECRET_KEY"

# Generate a unique folder name using timestamp
ISSUE_ID="issue-$(date "+%Y-%m-%d_%H-%M")"

# Upload files
mc cp -r /upload "manticore/$S3_BUCKET/$ISSUE_ID"

UPLOAD_PATH="s3://s3.manticoresearch.com/$S3_BUCKET/$ISSUE_ID"

echo "Upload complete. Please share the following path with the developers:"
echo "$UPLOAD_PATH"
