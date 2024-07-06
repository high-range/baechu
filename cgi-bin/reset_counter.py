#!/usr/bin/env python3
import os

print("Content-Type: text/plain")
print()

count_file_path = '/goinfre/sihlee/baechu/defence/counter.txt'  # 카운트 저장 파일 경로

# Initialize the counter
with open(count_file_path, 'w') as file:
    file.write('0')

print("Counter reset to 0")