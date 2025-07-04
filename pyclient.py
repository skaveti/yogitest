import socket

HOST = '192.168.1.129'  # Replace with your Pi's IP or hostname
PORT = 8080

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'Hi from Python client!')
    data = s.recv(1024)

print('Received from server:', data.decode())
