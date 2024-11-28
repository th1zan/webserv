#!/bin/bash

# Works with all_methods_allowed.conf

# Test server root URL and paths
SERVER="http://localhost:8080"
UPLOADLIB="../webSites/main/uploadlibrary"
UPLOADDIR="../webSites/main/upload"

# Helper function to print test results
function test_result {
  if [ $1 -eq 0 ]; then
    echo "✅ Test Passed: $2"
  else
    echo "❌ Test Failed: $2"
  fi
}

# Test cases
echo "Running tests..."

# --- GET Requests ---
curl -s -o /dev/null -w "%{http_code}" $SERVER/ | grep -q "200"
test_result $? "GET request to root returns 200"

curl -s -o /dev/null -w "%{http_code}" $SERVER/upload | grep -q "200"
test_result $? "GET request to upload endpoint returns 200"

curl -s -o /dev/null -w "%{http_code}" "$SERVER/?key=value" | grep -q "200"
test_result $? "GET request with query string returns 200"

curl -s -o /dev/null -w "%{http_code}" $SERVER/nonexistent | grep -q "404"
test_result $? "GET request to a non-existing file returns 404"

# --- POST Requests ---
curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: text/plain" --data "Test Data" $SERVER/upload | grep -q "200"
test_result $? "POST with valid Content-Type text/plain returns 200"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: application/json" --data '{"key":"value"}' $SERVER/upload | grep -q "200"
test_result $? "POST with JSON payload returns 200"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: application/json" --data-binary @$UPLOADLIB/sample.json $SERVER/upload/sample.json | grep -q "200"
test_result $? "POST with JSON payload returns 200"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: text/plain" --data-binary @$UPLOADLIB/sample.txt $SERVER/upload/sample.txt | grep -q "200"
test_result $? "POST with text file payload returns 200"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: image/png" --data-binary @$UPLOADLIB/sample.png $SERVER/upload/sample.png | grep -q "200"
test_result $? "POST with PNG image returns 200"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: application/pdf" --data-binary @$UPLOADLIB/sample.pdf $SERVER/upload/sample.pdf | grep -q "200"
test_result $? "POST with PDF file returns 200"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: text/plain" $SERVER/upload/empty_file.txt | grep -q "411"
test_result $? "POST with empty payload returns 411 Length Required"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: multipart/form-data; boundary=INVALIDBOUNDARY" --data-binary $'------INVALIDBOUNDARY\r\nContent-Disposition: form-data; name="file"; filename="test.txt"\r\nContent-Type: text/plain\r\n\r\nInvalid boundary content.\r\n------INVALIDBOUNDARY--' $SERVER/upload | grep -q "400"
test_result $? "POST with invalid boundary in multipart returns 400"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: application/x-unsupported" --data "Unsupported data" $SERVER/upload | grep -q "415"
test_result $? "POST with unsupported Content-Type returns 415"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary" --data-binary $'------WebKitFormBoundary\r\nContent-Disposition: form-data; name="file"; filename="single_file.txt"\r\nContent-Type: text/plain\r\n\r\nSingle file content.\r\n------WebKitFormBoundary--' $SERVER/upload | grep -q "200"
test_result $? "POST Multipart single file upload returns 200"

curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: multipart/form-data; boundary=----WebKitFormBoundary" --data-binary $'------WebKitFormBoundary\r\nContent-Disposition: form-data; name="file1"; filename="file1.txt"\r\nContent-Type: text/plain\r\n\r\nContent of file 1.\r\n------WebKitFormBoundary\r\nContent-Disposition: form-data; name="file2"; filename="file2.json"\r\nContent-Type: application/json\r\n\r\n{"key": "value"}\r\n------WebKitFormBoundary--' $SERVER/upload | grep -q "200"
test_result $? "POST Multipart multiple files upload returns 200"

# --- DELETE Requests ---
# Create file specifically for DELETE test
touch $UPLOADDIR/file_to_delete.txt

curl -s -o /dev/null -w "%{http_code}" -X DELETE $SERVER/upload/file_to_delete.txt | grep -q "200"
test_result $? "DELETE request for an existing file returns 200"

curl -s -o /dev/null -w "%{http_code}" -X DELETE $SERVER/upload/nonexistent.txt | grep -q "404"
test_result $? "DELETE request for a non-existing file returns 404"

curl -s -o /dev/null -w "%{http_code}" -X DELETE $SERVER/ | grep -q "403"
test_result $? "Invalid DELETE request to root returns 403"

# --- Error Triggers ---
curl -s -o /dev/null -w "%{http_code}" -X UNKNOWN $SERVER/ | grep -q "501"
test_result $? "Invalid HTTP method returns 501"

curl -s -o /dev/null -w "%{http_code}" --http1.1 --header "" $SERVER/ | grep -q "400"
test_result $? "Missing required headers return 400"

curl -s -o /dev/null -w "%{http_code}" "$SERVER/invalid|char" | grep -q "400"
test_result $? "Invalid URL characters return 400"

curl -s -o /dev/null -w "%{http_code}" -X GET $SERVER/ --http1.1 | grep -q "200"
test_result $? "Valid HTTP version (HTTP/1.1) returns 200"

curl -s -o /dev/null -w "%{http_code}" -X GET $SERVER/ --http1.0 | grep -q "505"
test_result $? "Invalid HTTP version (HTTP/1.0) returns 505"

# --- Cleanup Section ---
# Commented out to inspect test results
rm -f $UPLOADDIR/sample.*
rm -f $UPLOADDIR/upload
# rm -f $UPLOADDIR/file_to_delete.txt

echo "All tests completed!"

