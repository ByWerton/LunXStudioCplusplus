FROM ubuntu:22.04

# Derleyici araçlarını kuruyoruz
RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Depodaki her şeyi sunucuya kopyala
COPY . .

# index.cpp kullanacaksan burayı index.cpp yap, main ise main.cpp yap
RUN g++ -O3 main.cpp -o my_server

EXPOSE 10000

CMD ["./my_server"]
