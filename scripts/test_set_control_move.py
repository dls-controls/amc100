#!/usr/bin/env dls-python3
import socket


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("172.23.86.169", 9090))
    msg = b'{"jsonrpc": "2.0", "method": "com.attocube.amc.control.setControlMove", "params": [0, true], "id": 14}'
    sock.send(msg)
    print(sock.recv(256))
    sock.close()


if __name__ == "__main__":
    main()
