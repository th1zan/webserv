#!/usr/bin/env python3


import cgi
import warnings
# warnings.filterwarnings("ignore", category=DeprecationWarning)
warnings.filterwarnings("ignore")
# # Enable CGI debugging
# cgitb.enable()

# Parse the form data
form = cgi.FieldStorage()
year_of_birth = form.getvalue('year_of_birth')

# Check if input is valid and calculate age
if year_of_birth and year_of_birth.isdigit():
    age = 2024 - int(year_of_birth)
    result_message = f"Your age is: {age}"
else:
    result_message = "Invalid input! Please enter a valid year of birth."
    # Trigger a 400 Bad Request and let the server handle the custom error page
    # print("Status: 400 Bad Request")
    # print("Content-type:text/html")
    # print()
    # exit()
# Output the headers and HTML
# print("Content-type:text/html")
print()
print("<html>")
print("<head>")
print("<title>Age Calculator Result</title>")
print("<style>")
print("body { font-family: Arial, sans-serif; background-color: #f4f4f9; color: #333; margin: 0; padding: 20px; text-align: center; }")
print("h1 { color: #007BFF; }")
print(".container { max-width: 600px; margin: 50px auto; padding: 20px; background: #fff; border-radius: 8px; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1); }")
print("footer { margin-top: 20px; font-size: 14px; color: #666; }")
print("</style>")
print("</head>")
print("<body>")
print("<div class='container'>")
print(f"<h1>{result_message}</h1>")
print("</div>")
print("<footer>&copy; 2024 Age Calculator</footer>")
print("</body>")
print("</html>")
