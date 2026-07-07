#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct Vector3 {
    float x, y, z;
};

struct Vector2 {
    int x, y;
};

// Çözünürlüğü Render'ı yormayacak ama net görünecek şekilde optimize ettik
const int WIDTH = 80;
const int HEIGHT = 30;

Vector2 Project(Vector3 p, float angleX, float angleY) {
    // X ekseninde döndürme
    float radX = angleX * M_PI / 180.0f;
    float r1y = p.y * std::cos(radX) - p.z * std::sin(radX);
    float r1z = p.y * std::sin(radX) + p.z * std::cos(radX);

    // Y ekseninde döndürme
    float radY = angleY * M_PI / 180.0f;
    float rotX = p.x * std::cos(radY) - r1z * std::sin(radY);
    float rotZ = p.x * std::sin(radY) + r1z * std::cos(radY);

    float distance = 3.5f;
    float fov = 45.0f;

    float projectedX = (rotX * fov) / (rotZ + distance) + (WIDTH / 2);
    float projectedY = (r1y * fov) / (rotZ + distance) + (HEIGHT / 2);

    return Vector2{(int)projectedX, (int)projectedY};
}

int main() {
    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::stoi(port_env) : 10000;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return 1;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) return 1;
    if (listen(server_fd, 20) < 0) return 1;

    std::cout << "EphyXDrive Engine " << port << " portunda stabil modda aktif!" << std::endl;

    std::vector<Vector3> cubeVertices = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
    };

    std::vector<std::pair<int, int>> cubeEdges = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    float angleX = 0.0f;
    float angleY = 0.0f;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd >= 0) {
            // Açıyı her istekte kontrollü arttırıyoruz
            angleX += 5.0f;
            angleY += 7.0f;
            if (angleX >= 360.0f) angleX -= 360.0f;
            if (angleY >= 360.0f) angleY -= 360.0f;

            std::vector<std::string> buffer(HEIGHT, std::string(WIDTH, ' '));
            std::vector<Vector2> projectedPoints;
            
            for (const auto& vertex : cubeVertices) {
                projectedPoints.push_back(Project(vertex, angleX, angleY));
            }

            for (const auto& edge : cubeEdges) {
                Vector2 p1 = projectedPoints[edge.first];
                Vector2 p2 = projectedPoints[edge.second];

                int dx = p2.x - p1.x;
                int dy = p2.y - p1.y;
                int steps = std::max(std::abs(dx), std::abs(dy));

                float xInc = steps == 0 ? 0 : dx / (float)steps;
                float yInc = steps == 0 ? 0 : dy / (float)steps;

                float x = p1.x;
                float y = p1.y;

                for (int i = 0; i <= steps; i++) {
                    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                        buffer[(int)y][(int)x] = '#';
                    }
                    x += xInc;
                    y += yInc;
                }
            }

            std::string engineRenderOutput = "";
            for (int i = 0; i < HEIGHT; i++) {
                engineRenderOutput += buffer[i] + "\\n"; // JS içine basılacağı için kaçış karakteri ekledik
            }

            // Sayfa yenileme bitti! JavaScript motoru saniyede 30 kez arka planda istek atacak
            std::string html_content = 
                "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                "<style>body{background:#0a0a0a;color:#00ff66;font-family:monospace;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;margin:0;overflow:hidden;}"
                "pre{background:#111;padding:20px;border:1px solid #00ff66;box-shadow:0 0 15px rgba(0,255,102,0.3);border-radius:5px;line-height:1.2;font-size:14px;}</style></head>"
                "<body>"
                "<h2>EphyXDrive 3D Engine v0.02 Pre-Alpha</h2>"
                "<pre id='render'></pre>"
                "<p>Durum: Sunucu Baglantisi Akici</p>"
                "<script>"
                "function updateFrame() {"
                "  fetch(window.location.href, {headers: {'X-Requested-With': 'XMLHttpRequest'}})"
                "    .then(r => r.text())"
                "    .then(data => {"
                "      if(data.get) { document.getElementById('render').textContent = data; }"
                "    }).catch(() => {});"
                "}"
                "// Sayfayi yenilemeden arka planda veriyi ceker"
                "setInterval(() => {"
                "  fetch(window.location.pathname, {headers:{'X-Engine-Fetch':'true'}})"
                "  .then(res => res.text())"
                "  .then(text => { document.getElementById('render').textContent = text; });"
                "}, 100);" // 100ms araliklarla akici yenileme
                "</script>"
                "</body></html>";

            // Eğer istek arka plandaki JavaScript'ten geliyorsa sadece ham 3D metni gönder
            char header_buffer[1024] = {0};
            recv(client_fd, header_buffer, 1024, 0);
            std::string headers(header_buffer);

            if (headers.find("X-Engine-Fetch") != std::string::npos) {
                std::string clean_matrix = "";
                for (int i = 0; i < HEIGHT; i++) {
                    clean_matrix += buffer[i] + "\n";
                }
                std::string response = "HTTP/1.1 200 OK\r\n"
                                       "Content-Type: text/plain; charset=UTF-8\r\n"
                                       "Content-Length: " + std::to_string(clean_matrix.length()) + "\r\n"
                                       "Connection: close\r\n\r\n" + clean_matrix;
                send(client_fd, response.c_str(), response.length(), 0);
            } else {
                // Normal kullanıcı ilk kez girdiğinde arayüzü gönder
                std::string response = "HTTP/1.1 200 OK\r\n"
                                       "Content-Type: text/html; charset=UTF-8\r\n"
                                       "Content-Length: " + std::to_string(html_content.length()) + "\r\n"
                                       "Connection: close\r\n\r\n" + html_content;
                send(client_fd, response.c_str(), response.length(), 0);
            }
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
