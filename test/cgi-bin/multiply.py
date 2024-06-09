#!/usr/bin/env python3

import cgi

form = cgi.FieldStorage()

a = form.getvalue("a")
b = form.getvalue("b")

result = int(a) * int(b)

print(
    f"""
Content-type: text/plain

Result: {result}
"""
)
