import socket
import configparser
import os

def handle_client():
    config = configparser.ConfigParser()
    config_path = os.path.join(os.path.dirname(__file__), 'client_config.ini')
    config.read(config_path)

    HOST = config.get('SERVER', 'HOST', fallback='192.168.1.129')
    PORT = config.getint('SERVER', 'PORT', fallback=8080)
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            s.sendall(b'Hi from Python client!')
            data = s.recv(1024)

        print('Received from server:', data.decode())
    except KeyboardInterrupt:
        print("Client interrupted by user.")
    except Exception as e:
        print(f"An error occurred: {e}")
    except socket.error as e:
        print(f"Socket error: {e}")
    finally:
        s.close()
        print("Socket closed.")

if __name__ == "__main__":
    print("Client is running. Press Ctrl+C to exit.")
    handle_client()
