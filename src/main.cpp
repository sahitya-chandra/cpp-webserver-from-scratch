#include <iostream>       // cout, cerr
#include <sys/socket.h>   // for socket(), bind(), listen(), accept()
#include <netinet/in.h>   // for sockaddr_in
#include <unistd.h>       // for close()
#include <cstring>        // for memset
#include <sstream>        // for istringstream
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

    // get raw HTTP request
    constexpr size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE] = {0};

    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE-1);

    if (bytes_read < 0) {
        cerr << "Failed to read from client\n";
        return 1;
    }

    cout << "Received:\n" << buffer << "\n";

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

    string body;
    string status_line;
    string content_type = "text/plain";

    if(path == "/") {
        body = "Welcome to the Homepage";
        status_line = "HTTP/1.1 200 OK";
    } else if (path == "/about") {
        body = "This is the about page";
        status_line = "HTTP/1.1 200 OK";
    } else {
        body = "404 Not Found";
        status_line = "HTTP/1.1 404 Not Found";
    }

    ostringstream response_stream;
    response_stream << status_line << "\r\n"
                << "Content-Type: " << content_type << "\r\n"
                << "Content-Length: " << body.length() << "\r\n"
                << "\r\n"
                << body;

    string response = response_stream.str();
    send(client_fd, response.c_str(), response.length(), 0);

    // HTTP response
    const char* http_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, world!";

    ssize_t bytes_sent = send(client_fd, http_response, strlen(http_response), 0);

    if (bytes_sent < 0) {
        cerr << "Failed to send response\n";
        return 1;
    }

    cout << "Sent HTTP response\n";

    // close
    close(client_fd);
    close(server_fd);
}
