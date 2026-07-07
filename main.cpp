#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    // Render'ın atadığı portu oku, yoksa varsayılan 10000 yap
    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::stoi(port_env) : 10000;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Soket olusturulamadi!" << std::endl;
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Porta baglanilamadi!" << std::endl;
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Dinleme basarisiz!" << std::endl;
        return 1;
    }

    std::cout << "C++ Sunucusu " << port << " portunda canavar gibi calisiyor..." << std::endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd >= 0) {
            // Tarayıcıya gönderilecek basit HTTP cevabı
            std::string html_content = "<h1>EphyXDrive C++ Sunucusuna Hos Geldiniz!</h1><p>Anlik sınır yok, ozgurluk var.</p>";
            std::string response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html; charset=UTF-8\r\n"
                                   "Content-Length: " + std::to_string(html_content.length()) + "\r\n"
                                   "Connection: close\r\n\r\n" + html_content;
            
            send(client_fd, response.c_str(), response.length(), 0);
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
