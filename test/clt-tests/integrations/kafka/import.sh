#!/bin/bash
/opt/kafka/bin/kafka-console-producer.sh \
  --bootstrap-server localhost:9092 \
  --topic my-data \
  --property parse.key=true \
  --property key.separator=: \
  < /tmp/dump.json
