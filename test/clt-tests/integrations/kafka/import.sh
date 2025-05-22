#!/bin/bash

bootstrap_servers='localhost:9092'
kafka_topic='my-data'
json_file_path='/tmp/dump.json'


/opt/bitnami/kafka/bin/kafka-console-producer.sh \
  --broker-list "$bootstrap_servers" \
  --property parse.key=true \
  --property "key.separator=:" \
  --topic "$kafka_topic" < "$json_file_path"
