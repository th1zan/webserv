## How Webserv Works: Quick Overview

There are three main steps:

1. **Parse** a `.conf` file to get the settings for each server (there can be multiple servers). For example, the listening address and port are in `server.conf`.
2. **Setup** the server sockets. Each socket is linked to a port. This involves creating listening sockets for each server and adding them to a "polling list" to detect when a client connects to a socket.
3. **Launch** the servers. This means starting an infinite loop that:
   - Waits for the polling list to report an event to the system.
   - Reacts to that event.
   
   This loop includes client-server communication, using HTML methods (GET, POST, DELETE), and CGI.

![alt text](/docs/img/Overview.svg)

## Project Structure

Before detailing the steps, it's important to understand the project structure.

The project has four classes:

- **Service**: This class is instantiated first. It's central as it links the Parsing/Setup/Launch steps and monitors the sockets. It contains:
  - A list of servers (a vector).
  - A list of clients (a vector).
  - A polling list for sockets (a vector).
- **Parser**: Only one instance is created by the Service instance. It contains all parsing functions and temporary variables to create Server instances.
- **Server**: The `.conf` file contains server "blocks" (`server { ... }` with parameters). An instance of the Server class is created for each block and contains parsed parameters.
- **Client**: Each time a client connects to the server, a Client instance is created. It contains information about the client, including:
  - The server to which the client is connected.
  - The client's socket.
  - The client's request.

![](/docs/img/Classes.svg)

## In Detail, Closer to the code

### Step 1: Parsing

1. Main creates `Service` with `server.conf`.
2. `Service` constructor creates a `Parser` that reads the `.conf` file to analyze each `server{}` block.
3. `Parser` creates a `Server` instance for each `server{}` block. These instances are stored in `serverVector`.
4. `Service` retrieves `serverVector` from `Parser`.

For more explanations, see [Parser class explanations](Parsing-‐-Parser-class.md) and [the server directives](List-of-directives-and-verifications.md).

### Step 2: Setup

1. `main` calls the `setup()` method of `Service`.
2. For **each server** in `serverVector`, `Service` instance executes:
   - `getaddrinfo()` to get server address info.
   - `socket()` to create a socket.
   - `bind` to link the socket to a port.
   - `listen()` to set the socket to listening mode.
   - Adds the socket to `pollingFdVector` to monitor it later.

![](/docs/img/Setup.svg)

### Step 3: Launch

1. `main` calls the `launch()` method of `Service`.
2. `launch()` runs **in a loop** (until a shutdown signal) and:
   - `poll()` to monitor sockets (specifically file descriptors linked to sockets).
   - `poll()` takes `pollingFdVector` as input and blocks.
   - `poll()` unblocks when a socket reports an event or a timeout is reached.
   - `poll()` unblocks but **doesn't specify which socket or event type occurred**.
   - `PollingManager()` checks sockets to determine which socket triggered which event to handle data sending or receiving.

![](/docs/img/Launch.svg)

#### Step 3: PollingManager() - Socket Event Management

Three event types can be reported by a socket: error, ready to read, ready to write.

A. If a socket has an **error** (_POLLERR_, _POLLHUP_, _POLLNVAL_), the error is handled with an error message and connection closure.

B. If a socket is ready for **data reading** (_POLLIN_), it has data in its buffer. Action depends on socket type:
   - For a **server** socket (listening for connections, set with `listen()`), the connection is `accept()`.
     - `accept()` returns a new socket (file descriptor) for the new client.
     - This file descriptor is added to `pollingFdVector` for `poll()` to monitor.
     - A new `Client` instance is created.
   - For a **client** socket (representing an existing connection), data sent by the client is read with `recv()` and added to the `Client` instance's `_request` variable.

C. If a server socket is ready for **data sending** (_POLLOUT_), the buffer is empty and ready to be filled with information to send to the client.

![](/docs/img/pollingManager.svg)

#### Step 3: Request Processing and Sending Data

The process is as follows:

1. The client sends an HTML request, for example:

   ```
   GET /index.html HTTP/2
   Host: www.example.com
   ```

2. The server parses the request (`_request`) to identify:
   - The host (`Host`). It verifies if the host exists in the `serversVector`.
   - The method (GET, POST, DELETE).

3. The server then sends a response, which includes:
   - **Status**: A code indicating the result of the request (e.g., 200 OK, 404 Not Found).
   - **Headers**: Information about the response (content type, date, etc.).
   - **Body**: The data returned by the server (HTML, JSON, etc.).

**Example of an HTTP response:**

```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 138

<html>
<body>
<h1>Hello, World!</h1>
</body>
</html>
```

![HTTP request](/docs/img/HTTP_request.svg)




