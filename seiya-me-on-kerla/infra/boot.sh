#!/bin/bash
set -xue

while [[ ! -f /usr/local/bin/boot.py ]]; do
    echo "Waiting for Terraform to install /usr/local/bin/boot.py..."
    sleep 10
done

touch /var/log/boot.log
chmod +x /usr/local/bin/boot.py

sleep 10

echo "started at $(date) [$0]" >> /var/log/boot.log
python3 /usr/local/bin/boot.py  >> /var/log/boot.log 2>&1
echo "finished at $(date)" >> /var/log/boot.log
