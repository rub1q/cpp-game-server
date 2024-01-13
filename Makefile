.PHONY: build run clean install

SHELL = /bin/sh

CMAKE_COMMAND = /usr/bin/cmake
APP_NAME = game_server
CONFIG_PATH = ~/Projects/C++/cpp-backend/game-server/data/config.json
ROOT_PATH = ~/Projects/C++/cpp-backend/game-server/static
BUILD_TYPE = Debug

MAKEFLAGS += --silent

build:
	cd ./build/${BUILD_TYPE} && ${CMAKE_COMMAND} --build .

exec:
	./build/${BUILD_TYPE}/${APP_NAME} -c ${CONFIG_PATH} -w ${ROOT_PATH} -t 100 --randomize-spawn-points

run: build exec

clean:
	cd ./build/${BUILD_TYPE} && rm -rf ./bin/${APP_NAME}

install: 
	(mkdir ./build || true) && (mkdir ./build/${BUILD_TYPE} || true) && \
cd ./build/${BUILD_TYPE} && \
conan install ../.. --build=missing -s compiler.libcxx=libstdc++11 -s build_type=${BUILD_TYPE} && \
${CMAKE_COMMAND} ../.. -DCMAKE_BUILD_TYPE=${BUILD_TYPE}