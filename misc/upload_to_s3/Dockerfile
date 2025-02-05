# Use a minimal base image with curl and bash
FROM alpine:latest

# Install Minio client
RUN apk add --no-cache curl && \
    curl -o /usr/local/bin/mc https://dl.min.io/client/mc/release/linux-amd64/mc && \
    chmod +x /usr/local/bin/mc

# Set up Minio client and upload script
WORKDIR /app
COPY upload.sh /app/upload.sh
RUN chmod +x /app/upload.sh

# Define entrypoint script
ENTRYPOINT ["/app/upload.sh"]

# docker push manticoresearch/upload
