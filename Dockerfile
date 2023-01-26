# Build: docker build -t mdb.backend .

# Create Database:
# cd into the folder where the txt to be imported is (created db folder will be there too)
# docker run --volume "$(pwd)":/data mdb.backend /MillenniumDB/build/Release/bin/create_db /data/example-db.txt /data/docker_example

# Run server:
# docker run --volume "$(pwd)":/data --network="host" mdb.backend /MillenniumDB/build/Release/bin/server /data/docker_example

FROM alpine:edge

RUN apk --no-cache add cmake make g++
RUN apk --no-cache add openssl-dev
RUN apk --no-cache add boost1.81-dev

COPY ./src /MillenniumDB/src
COPY ./third_party/antlr4-runtime-4.9.3 /MillenniumDB/third_party/antlr4-runtime-4.9.3
COPY ./CMakeLists.txt /MillenniumDB/CMakeLists.txt

WORKDIR /MillenniumDB

RUN cmake -H. -Bbuild/Release -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build/Release/

RUN apk --no-cache add musl-locales

VOLUME ["/data"]
