#!/usr/bin/env python3

import cgi
import cgitb
cgitb.enable()

print("Content-Type: text/html")
print()
print("<html><head>")
print("<title>Form Submission</title>")
print("</head><body>")
print("<h1>Form Submission Result</h1>")

form = cgi.FieldStorage()

if "name" not in form or "email" not in form or "message" not in form:
    print("<p>Error: All fields are required.</p>")
else:
    name = form.getvalue("name")
    email = form.getvalue("email")
    message = form.getvalue("message")

    print("<p>Name: {}</p>".format(name))
    print("<p>Email: {}</p>".format(email))
    print("<p>Message: {}</p>".format(message))
    print("<p>Form submitted successfully!</p>")

print("</body></html>")
