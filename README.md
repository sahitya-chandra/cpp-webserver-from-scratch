# HTTP server from scratch

A simple static file web server built from scratch in C++ using low-level socket programming.  
This project demonstrates how web servers work under the hood by manually handling TCP connections and parsing HTTP requests.

---

## 🌐 Features

- ✅ Serves static files from the `public/` directory
- ✅ Parses raw HTTP GET requests
- ✅ Dynamically detects and serves correct MIME types
- ✅ Custom 404 page support (`public/404.html`)
- ✅ Handles multiple clients using `fork()`
- ✅ Written with only standard C/C++ libraries (no external dependencies)

---

## 📁 File Structure

```
http-server-from-scratch/
├── server.cpp # Core server logic
├── public/ # Static files served by the server
│ ├── index.html
│ ├── style.css
│ └── 404.html
├── Makefile # Build instructions
└── README.md # This file
```

---

## ⚙️ Build Instructions

### ✅ Requirements

- Linux (e.g., Ubuntu)
- g++ compiler

### 🛠️ Build & Run

```bash
make
./server
```

### Then open your browser or run:
```bash
curl http://localhost:8080/
```

---

## How It Works:
- Server binds to port 8080
- Listens for TCP connections
- Forks a child process to handle each incoming client
- Reads the HTTP request line
- Extracts the file path and serves it if available
- Sets appropriate Content-Type based on file extension
- Returns a 404 page if the file isn’t found

---

## Future Enhancements
- Support HTTP POST and other methods
- Add threading instead of fork() for concurrency
- Log requests and status codes
- Add command-line options for port or root directory
- HTTP/1.1 features (Keep-Alive, headers)

---

Built as a learning project to understand low-level networking and HTTP.
Feel free to fork, modify, or suggest improvements!

