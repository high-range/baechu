#!/usr/bin/env python3

import cgi

print("Content-Type: text/html")
print()

form = cgi.FieldStorage()

a = int(form.getvalue("a", 0))
b = int(form.getvalue("b", 0))

print("<html>")
print("<head><title>Multiplication Result</title></head>")
print("<body>")
print("<h1>Multiplication Result:</h1>")
print("<p>{} * {} = {}</p>".format(a, b, a * b))
print("</body>")
print("</html>")
