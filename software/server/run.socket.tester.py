#!/usr/bin/env python3
#
# Listens on a specified port and dumps received data in ASCII and hex.
#
# Args:
#    host (str): The host to bind to. Use '0.0.0.0' for all interfaces.
#    port (int): The port to listen on.
#
import sys
import socket
import argparse


BUFFER    = ""
LINE_SIZE = 16
streamInput = []


# Handles an incoming TCP connection, receiving and printing data.
def tcpHandleConnection(conn, addr):
    global BUFFER
    BUFFER = ""
    print(f"> Connected    {addr}")
    conn.setsockopt(socket.SOL_SOCKET,  socket.SO_KEEPALIVE, 1)     # After 1 second of inactivity, start sending probes
    conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPIDLE, 1)     # Send a probe every 3 seconds
    conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPCNT, 5)      # If a probe is not acknowledged, resend it up to 5 times
    conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPINTVL, 3)    # Time between individual keepalive probes
    with conn:
        newline = ''
        while True:
            try:
                data = conn.recv(1024)
                if not data:
                    print(f"{newline}> Disconnected {addr}")
                    return
                dataDump(data, addr)
                newline = '\n'
            except (socket.error, ConnectionResetError) as e:
                print(f"{newline}> Disconnected {addr}  ({e})")
                return

# Listens for TCP connections on a specified port and dumps received data.
def tcpServer(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen()
        while True:
            conn, addr = s.accept()
            tcpHandleConnection(conn, addr)

# Listens for UDP packets on a specified port and dumps received data.
def udpServer(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))
        while True:
            data, addr = s.recvfrom(1024)
            if not data:
                return
            dataDump(data, addr)

# Prints received data in both ASCII and hexadecimal formats.
def dataDump(data, addr):
    global BUFFER
    BUFFER += data.decode('utf-8')
    while len(BUFFER) > LINE_SIZE:
        dataDumpPrint()
        BUFFER = BUFFER[LINE_SIZE:]
        print("", flush=True)
    dataDumpPrint()

def dataDumpPrint():
    global BUFFER
    line = BUFFER[:LINE_SIZE]
    i = 1
    hexString = ''
    for char in line:
        hexString += f'{ord(char):02X} '
        if i % 8 == 0:
            hexString += ' '
        i += 1
    hexString += ((LINE_SIZE//8 + LINE_SIZE*3) - len(hexString)) * ' '
    asciiString = [char if ' '<=char<='~' else '.' for char in line]
    asciiString = "".join(asciiString) + (LINE_SIZE-len(line))*' '
    print(f"\r{hexString} |{asciiString}| ", end='', flush=True)



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Network listener and traffic dumper")
    parser.add_argument("-p", "--port", type=int, default=9100,  help=f"Port number to listen on  (default {9100})")
    parser.add_argument("-t", "--type", type=str, default='tcp', choices=['tcp', 'udp'], help="Connection type: tcp, udp  (default tcp)")
    parser.add_argument("-s", "--size", type=int, default=LINE_SIZE,  help=f"Screen column width       (default {LINE_SIZE})")
    args = parser.parse_args()
    HOST = '0.0.0.0'  # Listen on all interfaces
    LINE_SIZE = args.size
    try:
        print(f"\nNetwork listener                         Connection: {args.type}, Port: {args.port}\n")
        print(f"------------------------------------------------ Ctrl+C to abort ---\n")
        if args.type   == 'tcp':
            tcpServer(HOST, args.port)
        elif args.type == 'udp':
            udpServer(HOST, args.port)
    except KeyboardInterrupt:
        print("\n\nShutting down server, closing application\n")
