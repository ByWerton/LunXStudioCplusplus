FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN g++ -O3 main.cpp -o 3d_engine

EXPOSE 10000

CMD ["./3d_engine"]
