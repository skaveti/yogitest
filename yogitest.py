import time
import pigpio

def main():
    ip_addr = '192.168.1.129'
    pi = pigpio.pi(ip_addr)
    if not pi.connected:
        print("Failed to connect to pigpio daemon.")
        return

    try:
        print("Running main function...")
        pi.set_mode(16, pigpio.OUTPUT)  # Set GPIO 16 as output
        pi.write(16, True)  # Set GPIO 16 high
        # Add your main logic here
        time.sleep(5)  # Example delay
        pi.write(16, False)  # Set GPIO 16 low
    finally:
        pi.stop()
        print("Disconnected from pigpio daemon.")

if __name__ == "__main__":
    main()