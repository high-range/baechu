import os
import sys
import subprocess

print("Content-Type: text/plain")  # Content-Type을 text/plain으로 설정
print()

# 현재 작업 디렉터리 가져오기
base_dir = subprocess.check_output(['pwd'], universal_newlines=True).strip()
count_file_path = os.path.join(base_dir, 'html', 'submit', 'counter.txt')  # 카운트 저장 파일 경로
count_dir = os.path.dirname(count_file_path)

# Create the directory if it doesn't exist
if not os.path.exists(count_dir):
    os.makedirs(count_dir)
    sys.stderr.write(f"Created directory: {count_dir}\n")

# Reset the counter
with open(count_file_path, 'w') as file:
    file.write('0')

# Return the reset count
print(0)
