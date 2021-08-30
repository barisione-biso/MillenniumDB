FROM alpine:3.14

RUN apk --no-cache add cmake make g++ boost-dev

COPY ./src /MillenniumDB/src
COPY ./CMakeLists.txt /MillenniumDB/CMakeLists.txt

WORKDIR /MillenniumDB

RUN cmake -H. -Bbuild/Release -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build/Release/
