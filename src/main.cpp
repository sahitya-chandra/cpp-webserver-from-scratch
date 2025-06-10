#include <iostream>       // cout, cerr
#include <sys/socket.h>   // for socket(), bind(), listen(), accept()
#include <netinet/in.h>   // for sockaddr_in
#include <unistd.h>       // for close()
#include <cstring>        // for memset
#include <sstream>        // for istringstream
#include <fcntl.h>
#include <sys/stat.h>
using namespace std;

int main() {
    int server_fd;

    // Create Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr << "Failed to create socket\n";
        return 1;
    }

    cout << "Socket created\n";

    // TCP Socket in TIME_WAIT State (allow reuse immediately)
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "setsockopt(SO_REUSEADDR) failed\n";
        return 1;
    }

    //bind to 8080
    sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed\n";
        return 1;
    }

    cout << "Socket bound to port 8080\n";

    // listen (queueing connections)
    if (listen(server_fd, 5) < 0) {
        cerr << "Listen failed\n";
        return 1;
    }

    cout << "Server is listening on port 8080...\n";

    // wait for a connection
    sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    int client_fd = accept(server_fd, (sockaddr*)&client_address, &client_len);
    if (client_fd < 0) {
        cerr << "Accept failed\n";
        return 1;
    }

    cout << "Accepted a connection!\n";

    // Buffer to hold incoming request
    constexpr size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE] = {0};
    
    // Reads bytes from the client’s request or get the raw HTTP request
    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE-1);
    if (bytes_read < 0) {
        cerr << "Failed to read from client\n";
        return 1;
    }

    cout << "Received:\n" << buffer << "\n";

    // parsing the HTTP request
    istringstream request_stream(buffer);
    string request_line;
    getline(request_stream, request_line);

    if(!request_line.empty() && request_line.back() == '\r') {
        request_line.pop_back();
    }

    istringstream line_stream(request_line);
    string method, path, version;
    line_stream >> method >> path >> version;

    cout << "Method: " << method << "\n";
    cout << "Path: " << path << "\n";
    cout << "Version: " << version << "\n";

    // URL-to-filesystem mapping logic
    // loading and serving static files from disk
    string requested_file = (path == "/") ? "public/index.html" : "public" + path;

    int file_fd = open(requested_file.c_str(), O_RDONLY);

    string body;
    string status_line;
    string content_type = "text/html"; //default

    if(file_fd == -1) {
        body = "404 Not Found";
        status_line = "HTTP/1.1 404 Not Found";
    } else {
        // Get file size
        struct stat file_stat;
        fstat(file_fd, &file_stat);
        size_t file_size = file_stat.st_size;

        // Read file contents
        char* file_buffer = new char[file_size];
        read(file_fd, file_buffer, file_size);
        body.assign(file_buffer, file_size);
        delete[] file_buffer;

        close(file_fd);
        status_line = "HTTP/1.1 200 OK";

        if (requested_file.ends_with(".css")) {
            content_type = "text/css";
        } else if (requested_file.ends_with(".js")) {
            content_type = "application/javascript";
        } else if (requested_file.ends_with(".txt")) {
            content_type = "text/plain";
        }
    }

    // routing using path
    // if(path == "/") {
    //     body = "Welcome to the Homepage";
    //     status_line = "HTTP/1.1 200 OK";
    // } else if (path == "/about") {
    //     body = "This is the about page";
    //     status_line = "HTTP/1.1 200 OK";
    // } else {
    //     body = "404 Not Found";
    //     status_line = "HTTP/1.1 404 Not Found";
    // }


    //build & send the res
    ostringstream response_stream;
    response_stream << status_line << "\r\n"
                << "Content-Type: " << content_type << "\r\n"
                << "Content-Length: " << body.length() << "\r\n"
                << "\r\n"
                << body;

    string response = response_stream.str();
    send(client_fd, response.c_str(), response.length(), 0);

    // send an HTTP response
    // const char* http_response =
    //     "HTTP/1.1 200 OK\r\n"
    //     "Content-Type: text/plain\r\n"
    //     "Content-Length: 13\r\n"
    //     "\r\n"
    //     "Hello, world!";

    // ssize_t bytes_sent = send(client_fd, http_response, strlen(http_response), 0);
    // if (bytes_sent < 0) {
    //     cerr << "Failed to send response\n";
    //     return 1;
    // }
    // cout << "Sent HTTP response\n";

    // close
    close(client_fd);
    close(server_fd);
}
