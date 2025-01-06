#!/usr/bin/env python3

import cgi
import warnings

# Suppress deprecation warnings
warnings.filterwarnings("ignore")

# Parse the form data
form = cgi.FieldStorage()
weight = form.getvalue('weight')  # Weight in kilograms
height = form.getvalue('height')  # Height in meters

# Function to calculate BMI
def calculate_bmi(weight, height):
    try:
        weight = float(weight)
        height = float(height) / 100  # Convert height to meters
        if weight > 0 and height > 0:
            return round(weight / (height ** 2), 2)
        else:
            return None
    except ValueError:
        return None

# Check if input is valid and calculate BMI
if weight and height:
    bmi = calculate_bmi(weight, height)
    if bmi is not None:
        if bmi < 18.5:
            category = "Underweight"
        elif 18.5 <= bmi < 24.9:
            category = "Normal weight"
        elif 25 <= bmi < 29.9:
            category = "Overweight"
        else:
            category = "Obesity"
        result_message = f"Your BMI is: {bmi} ({category})"
    else:
        result_message = "Invalid input! Weight and height must be positive numbers."
else:
    result_message = "Invalid input! Please enter your weight and height."

# Output the headers and HTML
print("Content-type:text/html")
print()
print("<html>")
print("<head>")
print("<title>BMI Calculator Result</title>")
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
print("<footer>&copy; 2024 BMI Calculator</footer>")
print("</body>")
print("</html>")