#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys
import json
import math

cgitb.enable()  # CGI 스크립트 디버깅 활성화

print("Content-Type: application/json")
print()

form = cgi.FieldStorage()

first_value = form.getvalue("first_value")
second_value = form.getvalue("second_value")
operator = form.getvalue("operator")

response = {}

if first_value is None or second_value is None or operator is None:
    response["error"] = "Missing parameters"
    print(json.dumps(response))
    sys.exit(1)

try:
    first_value = float(first_value)
    second_value = float(second_value)
except ValueError:
    response["error"] = "Invalid numbers"
    print(json.dumps(response))
    sys.exit(1)

if operator == '+':
    result = first_value + second_value
elif operator == '-':
    result = first_value - second_value
elif operator == '*':
    result = first_value * second_value
elif operator == '/':
    if second_value == 0:
        response["error"] = "Division by zero"
        print(json.dumps(response))
        sys.exit(1)
    result = first_value / second_value
else:
    response["error"] = "Invalid operator"
    print(json.dumps(response))
    sys.exit(1)

# NaN 또는 Inf 값 검사
if math.isnan(result) or math.isinf(result):
    response["error"] = "Calculation resulted in an invalid value"
    print(json.dumps(response))
    sys.exit(1)

# 결과가 정수인지 확인하여 정수로 표시
if result.is_integer():
    result = int(result)

response["result"] = result
print(json.dumps(response))
