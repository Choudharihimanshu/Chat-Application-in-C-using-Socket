# Multi-User Chat Application in C (Terminal-based)

A terminal-based chat application where multiple clients can communicate via a central server. The server uses threads to handle multiple users concurrently.

## Features
- Multiple clients
- Real-time message broadcasting
- Threaded client handling
- Pure C with sockets and pthreads

## 🛠️ Build Instructions

```bash
gcc server.c -o server -lpthread
gcc client.c -o client -lpthread
