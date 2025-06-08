#include <iostream>       // cout, cerr
#include <sys/socket.h>   // for socket(), bind(), listen(), accept()
#include <netinet/in.h>   // for sockaddr_in
#include <unistd.h>       // for close()
#include <cstring>        // for memset
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

    // listen
    if (listen(server_fd, 5) < 0) {
        cerr << "Listen failed\n";
        return 1;
    }

    cout << "Server is listening on port 8080...\n";

    sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    int client_fd = accept(server_fd, (sockaddr*)&client_address, &client_len);
    if (client_fd < 0) {
        cerr << "Accept failed\n";
        return 1;
    }

    cout << "Accepted a connection!\n";

    constexpr size_t BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE] = {0};

    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE-1);

    if (bytes_read < 0) {
        cerr << "Failed to read from client\n";
        return 1;
    }

    cout << "Received:\n" << buffer << "\n";

    // close
    close(client_fd);
    close(server_fd);
}
