#!/usr/bin/env python3
import cgi
import os
import json
import subprocess
from datetime import datetime

# 현재 작업 디렉토리 가져오기
base_dir = subprocess.check_output(['pwd'], universal_newlines=True).strip()
feedback_file_path = os.path.join(base_dir, 'html', 'submit', 'feedbacks.json')
feedback_dir = os.path.dirname(feedback_file_path)

# 피드백 디렉토리가 존재하지 않으면 생성
if not os.path.exists(feedback_dir):
    os.makedirs(feedback_dir)

# CGI 환경 설정
print("Content-Type: application/json")
print()

# 폼 데이터 파싱
form = cgi.FieldStorage()
name = form.getvalue('name')
message = form.getvalue('message')

# 현재 시간 가져오기
timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

# 새 피드백 데이터 생성
new_feedback = {
    'name': name,
    'message': message,
    'timestamp': timestamp
}

# 기존 피드백 데이터 로드 또는 초기화
feedbacks = []
if os.path.exists(feedback_file_path) and os.path.getsize(feedback_file_path) > 0:
    with open(feedback_file_path, 'r') as file:
        try:
            feedbacks = json.load(file)
        except json.JSONDecodeError:
            feedbacks = []

# 새 피드백 추가
feedbacks.append(new_feedback)

# 피드백 데이터 저장
with open(feedback_file_path, 'w') as file:
    json.dump(feedbacks, file)

# 응답 반환
print(json.dumps(new_feedback))
