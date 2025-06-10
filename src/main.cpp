#include <iostream>       // cout, cerr
#include <sys/socket.h>   // for socket(), bind(), listen(), accept()
#include <netinet/in.h>   // for sockaddr_in
#include <unistd.h>       // for close()
#include <cstring>        // for memset
#include <sstream>        // for istringstream
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include <unordered_map>
using namespace std;

string get_mime_type(const string& path, const unordered_map<string, string>& mime_types) {
    size_t dot_pos = path.rfind('.');
    if(dot_pos != string::npos) {
        string ext = path.substr(dot_pos);
        if(mime_types.count(ext)) {
            return mime_types.at(ext);
        }
    }
    return "application/octet-stream"; // default fallback
}

int main() {
    int port = 8080;
    unordered_map<string, string> mime_types = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".svg", "image/svg+xml"},
    {".json", "application/json"},
    {".txt", "text/plain"},
};


    // Create Socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
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
    address.sin_port = htons(port);

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
        status_line = "HTTP/1.1 404 Not Found";
        string not_found_file = "public/404.html";
        int nf_fd = open(not_found_file.c_str(), O_RDONLY);

        if(nf_fd != -1) {
            struct stat nf_stat;
            fstat(nf_fd, &nf_stat);
            size_t nf_size = nf_stat.st_size;

            char* nf_buffer = new char[nf_size];
            read(nf_fd, nf_buffer, nf_size);
            body.assign(nf_buffer, nf_size);
            delete[] nf_buffer;
            close(nf_fd);

            content_type = get_mime_type(not_found_file, mime_types);
        } else {
            body = "<h1>404 Not Found</h1>";
            content_type = "text/html";
        }
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

        content_type = get_mime_type(requested_file, mime_types);
    }


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
