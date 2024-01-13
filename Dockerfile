FROM ubuntu:22.04 AS build

RUN apt update && \
    apt upgrade -y && \
    apt install -y \
        python3-pip \
        build-essential \
        gcc-12 \
        g++-12 \
        cpp-12 \
        cmake \
    && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 12 \
    && \
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 12 \
    && \
    pip3 install conan==1.*

COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing 

COPY ./src /app/src/
COPY ./data /app/data/
COPY CMakeLists.txt .env /app/

RUN cd /app/ && \
    cd /app/build/ && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

####################################################

FROM ubuntu:22.04 AS run

RUN groupadd -r www && useradd -r -g www www
USER www

COPY --from=build /app/build/bin/game_server /app/
COPY ./data /app/data

ENTRYPOINT ["/app/game_server" "/app/data/config.json"]