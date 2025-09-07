FROM gcc:latest

RUN apt-get update && apt-get install -y cmake gdb

WORKDIR /server

COPY . .

RUN cmake .

RUN make

EXPOSE 8080

CMD ["./server"]
