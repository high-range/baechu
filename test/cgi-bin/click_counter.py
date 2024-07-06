#!/usr/bin/env python3
import cgi
import os

print("Content-Type: text/plain")  # Content-Type을 text/plain으로 변경
print()

count_file_path = '/goinfre/sihlee/baechu/test/defence/counter.txt'  # 카운트 저장 파일 경로

# Initialize the counter if the file doesn't exist
if not os.path.exists(count_file_path):
    with open(count_file_path, 'w') as file:
        file.write('0')

# Read the current count
with open(count_file_path, 'r') as file:
    content = file.read().strip()
    try:
        count = int(content)
    except ValueError:
        count = 0

# Increment the count
count += 1

# Save the new count
with open(count_file_path, 'w') as file:
    file.write(str(count))

# Return the updated count
print(count)
