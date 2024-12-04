#!/usr/bin/env python3

import cgi
import os
import cgitb
import os

cgitb.enable()

# Specify the directory to save uploaded files
UPLOAD_DIR = "../upload"

# Ensure the upload directory exists
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Get form data
form = cgi.FieldStorage()

try:
    # Check if the file field is present
    if "file" not in form:
        raise KeyError("No file field in the form!")

    file_field = form["file"]

    # Check if a file was uploaded
    if not file_field.filename:
        raise ValueError("No file uploaded!")

    # Get the uploaded file's name
    filename = os.path.basename(file_field.filename)

    # Save the file to the upload directory
    file_path = os.path.join(UPLOAD_DIR, filename)
    with open(file_path, "wb") as output_file:
        output_file.write(file_field.file.read())

    # Respond with a success message
    print("Content-type: text/html\n")
    print("<html>")
    print("<head><title>File Upload Success</title></head>")
    print("<body>")
    print(f"<h1>File '{filename}' uploaded successfully!</h1>")
    print(f"<p>Saved to: {file_path}</p>")
    print("</body>")
    print("</html>")

except KeyError as e:
    print("Content-type: text/html\n")
    print("<html>")
    print("<head><title>File Upload Failed</title></head>")
    print(f"<body><h1>Error: {str(e)}</h1></body>")
    print("</html>")
except ValueError as e:
    print("Content-type: text/html\n")
    print("<html>")
    print("<head><title>File Upload Failed</title></head>")
    print(f"<body><h1>Error: {str(e)}</h1></body>")
    print("</html>")
except Exception as e:
    print("Content-type: text/html\n")
    print("<html>")
    print("<head><title>File Upload Failed</title></head>")
    print("<body>")
    print(f"<h1>Unexpected Error</h1><p>{str(e)}</p>")
    print("</body>")
    print("</html>")

# #!/usr/bin/env python3

# import cgi
# import os
# import cgitb

# # Enable debugging
# cgitb.enable()

# # Specify the directory to save uploaded files
# UPLOAD_DIR = "../upload"  

# # Ensure the upload directory exists
# os.makedirs(UPLOAD_DIR, exist_ok=True)

# # Get form data
# form = cgi.FieldStorage()

# # Check if the file field is present
# if "file" in form:
#     file_field = form["file"]

#     # Check if a file was uploaded
#     if file_field.filename:
#         # Get the uploaded file's name
#         filename = os.path.basename(file_field.filename)

#         # Save the file to the upload directory
#         file_path = os.path.join(UPLOAD_DIR, filename)
#         with open(file_path, "wb") as output_file:
#             output_file.write(file_field.file.read())

#         # Respond with a success message
#         print("Content-type: text/html\n")
#         print("<html>")
#         print("<head><title>File Upload Success</title></head>")
#         print("<body>")
#         print(f"<h1>File '{filename}' uploaded successfully!</h1>")
#         print(f"<p>Saved to: {file_path}</p>")
#         print("</body>")
#         print("</html>")
#     else:
#         # No file was uploaded
#         print("Content-type: text/html\n")
#         print("<html>")
#         print("<head><title>File Upload Failed</title></head>")
#         print("<body><h1>No file uploaded!</h1></body>")
#         print("</html>")
# else:
#     # No file field in the request
#     print("Content-type: text/html\n")
#     print("<html>")
#     print("<head><title>File Upload Failed</title></head>")
#     print("<body><h1>No file field in the form!</h1></body>")
#     print("</html>")