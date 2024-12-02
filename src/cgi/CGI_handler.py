#!/usr/bin python3

import shutil
import os
import sys

def POST():
    try:
        # Ensure the argument is passed
        if len(sys.argv) < 2:
            raise ValueError("Binary data argument is missing.")
        
        binary_data = sys.argv[1]  # First argument
        dest_folder = "/home/joshua/Desktop/webserv/webSites/main/upload/"
        filename = os.environ.get('FILENAME')

        # Ensure FILENAME is set
        if not filename:
            raise ValueError("Environment variable 'FILENAME' is not set.")
        
        addFileBinary(binary_data, dest_folder, filename)

    except Exception as e:
        print("System Error: ", e)
        sys.exit(1)

def addFileBinary(binary_data, dest_folder_path, filename):
    try:
        # Ensure the destination folder exists
        if not os.path.exists(dest_folder_path):
            os.makedirs(dest_folder_path, exist_ok=True)
        
        # Full path to the temporary file
        temp_file_path = os.path.join(dest_folder_path, filename)

        # Write the binary data (open in binary write mode)
        with open(temp_file_path, 'wb') as temp_file:
            temp_file.write(binary_data.encode('utf-8'))  # Assuming binary data is passed as a UTF-8 string
        print(f"File written successfully to {temp_file_path}")

    except PermissionError:
        raise PermissionError(f"Permission denied: Unable to write to {dest_folder_path}")
    except Exception as e:
        raise RuntimeError(f"Failed to write the file: {e}")

def ErrorResponse():
    return "Invalid request method."

def GET():
    try:
        # Ensure the argument is passed
        if len(sys.argv) < 2:
            raise ValueError("File path argument is missing.")
        
        file_path = sys.argv[1]  # First argument
        if not os.path.exists(file_path):
            raise FileNotFoundError(f"File not found: {file_path}")
        
        with open(file_path, 'rb') as file:
            shutil.copyfileobj(file, sys.stdout.buffer)
    except Exception as e:
        return(ErrorResponse())

def handle_request(request):
    try:
        if request == "POST":
            POST()
            return "File uploaded successfully."
        elif request == "GET":
            GET()
            return "Invalid request method."
    except Exception as e:
        return f"System Error: {e}"

if __name__ == "__main__":
    request = sys.stdin.read()
    response = handle_request(request)
    sys.stdout.write(response)

