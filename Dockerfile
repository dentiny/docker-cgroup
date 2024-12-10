FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    g++ \
    build-essential \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY stall_forever.cc .

# Compile the C++ application
RUN g++ -o stall_forever stall_forever.cc

# Specify the command to run your program
CMD ["./stall_forever"]
