#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Basit Bir 3D Vektör Yapısı
struct Vector3 {
    float x, y, z;
};

// 2D Ekran Koordinatı Yapısı
struct Vector2 {
    int x, y;
};

// Sunucu Genişlik ve Yükseklik Ayarları (İşlemciyi zorlamak için arttırılabilir)
const int WIDTH = 120;
const int HEIGHT = 40;

// 3D Noktayı 2D Ekrana İzdüşürme (Perspective Projection) Matematiği
Vector2 Project(Vector3 p, float angle) {
    // Y ekseninde döndürme matrisi uygulaması
    float rad = angle * M_PI / 180.0f;
    float rotX = p.x * std::cos(rad) - p.z * std::sin(rad);
    float rotZ = p.x * std::sin(rad) + p.z * std::cos(rad);

    // Kamera mesafesi ayarı
    float distance = 3.5f;
    float fov = 60.0f;

    float projectedX = (rotX * fov) / (rotZ + distance) + (WIDTH / 2);
    float projectedY = (p.y * fov) / (rotZ + distance) + (HEIGHT / 2);

    return Vector2{(int)projectedX, (int)projectedY};
}

int main() {
    // Render Port Ayarı
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
    if (listen(server_fd, 10) < 0) return 1;

    std::cout << "3D Software Engine " << port << " portunda aktif!" << std::endl;

    // 3D Küpün Köşe Noktaları (Vertices)
    std::vector<Vector3> cubeVertices = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1}
    };

    // Küpün Kenar Çizgileri (Edges) - Noktaları birbirine bağlar
    std::vector<std::pair<int, int>> cubeEdges = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    float rotationAngle = 0.0f;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd >= 0) {
            // Her istek geldiğinde küpü biraz döndür (Sınır zorlama mekanizması)
            rotationAngle += 15.0f;
            if (rotationAngle >= 360.0f) rotationAngle -= 360.0f;

            // Arka Plan Tamponu (Frame Buffer) Oluşturma ve Temizleme
            std::vector<std::string> buffer(HEIGHT, std::string(WIDTH, ' '));

            // 3D Noktaları İzdüşür
            std::vector<Vector2> projectedPoints;
            for (const auto& vertex : cubeVertices) {
                projectedPoints.push_back(Project(vertex, rotationAngle));
            }

            // Rasterizasyon: Çizgileri Ekrana Nokta Olarak Basma
            for (const auto& edge : cubeEdges) {
                Vector2 p1 = projectedPoints[edge.first];
                Vector2 p2 = projectedPoints[edge.second];

                // Basit DDA Çizgi Çizme Algoritması
                int dx = p2.x - p1.x;
                int dy = p2.y - p1.y;
                int steps = std::max(std::abs(dx), std::abs(dy));

                float xInc = dx / (float)steps;
                float yInc = dy / (float)steps;

                float x = p1.x;
                float y = p1.y;

                for (int i = 0; i <= steps; i++) {
                    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                        buffer[(int)y][(int)x] = '#'; // Küpün gövde çizgisi karakteri
                    }
                    x += xInc;
                    y += yInc;
                }
            }

            // Ekrandaki ASCII karakterleri tek bir HTML metnine dönüştürme
            std::string engineRenderOutput = "";
            for (int i = 0; i < HEIGHT; i++) {
                engineRenderOutput += buffer[i] + "\n";
            }

            // Tarayıcıya Gönderilecek Dinamik Sayfa Yapısı
            std::string html_content = 
                "<html><head><meta http-equiv='refresh' content='0.1'>" // Saniyede 10 kere sayfayı yeniler (FPS sayacı gibi)
                "<style>body{background:#111;color:#0f0;font-family:monospace;white-space:pre;font-size:14px;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;margin:0;}</style></head>"
                "<body>"
                "<h2>EphyXDrive 3D Software Engine v0.01 Pre-Alpha</h2>"
                "<div>" + engineRenderOutput + "</div>"
                "<p>Aci: " + std::to_string((int)rotationAngle) + " derece - Tamamen CPU ile hesaplaniyor.</p>"
                "</body></html>";

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
