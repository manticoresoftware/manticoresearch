#!/bin/bash
set -e

total_iterations=100

output_replace="/tmp/insert_commands.sql"
echo "" > $output_replace

start_time2=$(date +%s%3N) && for ((i=1; i<=total_iterations; i++)); do
    echo "REPLACE INTO tbl (id, model, storage_capacity, color, release_year, price, discounted_price, sold, date_added, product_codes, values, additional_info, vector) VALUES
    (${i}101, 'iPhone 13 Pro', 256, 'silver${i}', 2021, 1099.99, 989.99, 'TRUE', '1591362342000', (1,2,3), (523456764345678976, 98765409877866654098, 1109876543450987650987), '{\"features\": [\"ProMotion display\", \"A15 Bionic chip\", \"Ceramic Shield front cover\"]}', (0.773448, 0.312478, 0.137971, 0.459821));" >> $output_replace

done

mysql -h0 -P9306 < $output_replace > /dev/null && end_time2=$(date +%s%3N) && export elapsed_time2=$((end_time2 - start_time2))
