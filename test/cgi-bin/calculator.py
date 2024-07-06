#!/usr/bin/env python3
import cgi
import cgitb
import json

cgitb.enable()

print("Content-Type: application/json")
print()

form = cgi.FieldStorage()
operation = form.getvalue("operation")
num1 = form.getvalue("num1")
num2 = form.getvalue("num2")

try:
    num1 = float(num1)
    num2 = float(num2)
    if operation == "add":
        result = num1 + num2
    elif operation == "subtract":
        result = num1 - num2
    elif operation == "multiply":
        result = num1 * num2
    elif operation == "divide":
        if num2 != 0:
            result = num1 / num2
        else:
            result = "Error: Division by zero"
    else:
        result = "Error: Invalid operation"
except (ValueError, TypeError):
    result = "Error: Invalid input"

print(json.dumps({"result": result}))
