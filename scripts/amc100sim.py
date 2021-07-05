#!/usr/bin/env python
import json
import socket
import threading
import time
RECV_SIZE = 512
N_AXES = 8

axes_pos = [0.0] * N_AXES
axes_output = [False] * N_AXES
axes_controlmove = [False] * N_AXES
axes_amp = [1000] * N_AXES
axes_freq = [50000] * N_AXES
axes_autoreset = [False] * N_AXES
axes_moving = [0] * N_AXES
axes_eotfwd = [False] * N_AXES
axes_eotbkwd = [False] * N_AXES


def send(sock, cmd, recv=False):
    print("Sent: {}".format(repr(cmd)))
    sock.send(cmd)
    if recv:
        print("Recv: {}".format(sock.recv(RECV_SIZE)))


def method_not_found(sock, msg):
    print(f"Method {msg['method']} not handled")


def delay_move(axis, pos):
    time.sleep(1)
    axes_pos[axis] = int(pos)
    axes_moving[axis] = 0


def com_attocube_amc_move_setControlTargetPosition(sock, msg):
    axis = msg["params"][0]
    pos = msg["params"][1]
    axes_moving[axis] = 1
    threading.Thread(None, delay_move, args=(axis, pos)).start()
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0]
    }).encode() + b'\n')


def com_attocube_amc_control_setControlMove(sock, msg):
    axis = msg["params"][0]
    enable = msg["params"][1]
    axes_controlmove[axis] = enable
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0]
    }).encode() + b'\n')


def com_attocube_amc_status_getStatusReference(sock, msg):
    axis = msg["params"][0]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, True]
    }).encode() + b'\n')


def com_attocube_amc_status_getStatusConnected(sock, msg):
    axis = msg["params"][0]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, True]
    }).encode() + b'\n')


def com_attocube_amc_control_getReferencePosition(sock, msg):
    axis = msg["params"][0]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, 0]
    }).encode() + b'\n')


def com_attocube_amc_status_getStatusEotFwd(sock, msg):
    axis = msg["params"][0]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, axes_eotfwd[axis]]
    }).encode() + b'\n')


def com_attocube_amc_status_getStatusEotBkwd(sock, msg):
    axis = msg["params"][0]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, axes_eotbkwd[axis]]
    }).encode() + b'\n')


def com_attocube_amc_status_getStatusMoving(sock, msg):
    axis = msg["params"][0]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, axes_moving[axis]]
    }).encode() + b'\n')


def com_attocube_amc_control_setControlAutoReset(sock, msg):
    axis = msg["params"][0]
    enable = msg["params"][1]
    axes_autoreset[axis] = enable
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0]
    }).encode() + b'\n')


def com_attocube_system_getFirmwareVersion(sock, msg):
    sock.send(json.dumps({
        "id": msg["id"],
        "result": ["1"]
    }).encode() + b'\n')


def com_attocube_amc_move_getPosition(sock, msg):
    axis = msg["params"][0]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, axes_pos[axis]]
    }).encode() + b'\n')


def com_attocube_amc_control_setControlOutput(sock, msg):
    axis = msg["params"][0]
    output = msg["params"][1]
    axes_output[axis] = output
    print(f"Setting {axis} output to {output}")
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0]
    }).encode() + b'\n')


def com_attocube_amc_control_getControlOutput(sock, msg):
    axis = msg["params"][0]
    output = axes_output[axis]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, output]
    }).encode() + b'\n')


def com_attocube_amc_control_setControlAmplitude(sock, msg):
    axis = msg["params"][0]
    amplitude = msg["params"][1]
    axes_amp[axis] = amplitude
    print(f"Setting {axis} amplitude to {amplitude} mV")
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0]
    }).encode() + b'\n')


def com_attocube_amc_control_getControlAmplitude(sock, msg):
    axis = msg["params"][0]
    amp = axes_amp[axis]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, amp]
    }).encode() + b'\n')


def com_attocube_amc_control_setControlFrequency(sock, msg):
    axis = msg["params"][0]
    freq = msg["params"][1]
    axes_freq[axis] = freq
    print(f"Setting {axis} freq to {freq} mH")
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0]
    }).encode() + b'\n')


def com_attocube_amc_control_getControlFrequency(sock, msg):
    axis = msg["params"][0]
    freq = axes_freq[axis]
    sock.send(json.dumps({
        "id": msg["id"],
        "result": [0, freq]
    }).encode() + b'\n')


def client_handler(sock, addr):
    while True:
        data = sock.recv(RECV_SIZE).decode()
        if not data:
            print(f"Closed {addr}")
            sock.close()
            return
        for data_part in data.split("\n"):
            if not data_part:
                continue
            try:
                msg = json.loads(data_part)
            except Exception as e:
                print(f"Couldn't decode {repr(data)}")
                print(f"{e}")
                continue
            method = msg.get("method")
            if not method:
                print(f"method attributte not found in {data}")
                continue
            globals().get(
                method.replace('.', '_'), method_not_found)(sock, msg)


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', 9090))
    sock.listen(5)
    while True:
        client_sock, client_addr = sock.accept()
        print(f"Got client {client_addr}")
        threading.Thread(target=client_handler,
                         args=(client_sock, client_addr)).start()
    sock.close()


if __name__ == "__main__":
    main()
