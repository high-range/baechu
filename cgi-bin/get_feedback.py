#!/usr/bin/env python3
import os
import json
import subprocess

# 현재 작업 디렉토리 가져오기
base_dir = subprocess.check_output(['pwd'], universal_newlines=True).strip()
feedback_file_path = os.path.join(base_dir, 'defence', 'submit', 'feedbacks.json')

print("Content-Type: application/json")
print()

# 기존 피드백 데이터 로드 또는 초기화
feedbacks = []
if os.path.exists(feedback_file_path) and os.path.getsize(feedback_file_path) > 0:
    with open(feedback_file_path, 'r') as file:
        try:
            feedbacks = json.load(file)
        except json.JSONDecodeError:
            feedbacks = []

# 응답 반환
print(json.dumps(feedbacks))
