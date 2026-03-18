#!/usr/bin/env python3
"""
Command line script to send start_signal struct over USB CDC.

Usage: python send_signal.py --port /dev/ttyACM0 --duration 1000 --speed 50 --bin metal
"""

import argparse
import struct
import serial


# Enum values matching C litter_type
LITTER_TYPES = {
    "metal": 0,
    "plastic": 1,
    "paper": 2,
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Send start_signal struct over UART"
    )
    parser.add_argument(
        "--port", "-p",
        required=True,
        help="Serial port (e.g., /dev/ttyUSB0 or COM3)"
    )
    parser.add_argument(
        "--duration", "-d",
        type=int,
        required=True,
        help="Duration value (int)"
    )
    parser.add_argument(
        "--speed", "-s",
        type=int,
        required=True,
        help="Speed value (int)"
    )
    parser.add_argument(
        "--bin", "-b",
        choices=["metal", "plastic", "glass"],
        required=True,
        help="Litter type: metal, plastic, or glass"
    )
    parser.add_argument(
        "--baudrate", "-r",
        type=int,
        default=115200,
        help="Baud rate (default: 115200)"
    )
    return parser.parse_args()


def pack_start_signal(speed: int, duration: int, litter: int) -> bytes:
    """
    Pack the start_signal struct.
    
    struct start_signal {
        int speed;          // 4 bytes
        int duration;       // 4 bytes
        litter_type litter; // 4 bytes (enum)
    };
    
    Using little-endian format.
    """
    return struct.pack('<iii', speed, duration, litter)


def send_signal(port: str, baudrate: int, speed: int, duration: int, litter: int):
    """Send the packed struct over UART."""
    data = pack_start_signal(speed, duration, litter)
    
    print(f"Connecting to {port} at {baudrate} baud...")
    print(f"Sending: speed={speed}, duration={duration}, litter={litter}")
    print(f"Raw bytes: {data.hex()}")
    
    with serial.Serial(port, baudrate, timeout=1) as ser:
        bytes_sent = ser.write(data)
        ser.flush()
        print(f"Sent {bytes_sent} bytes successfully")


def main():
    args = parse_args()
    
    litter_value = LITTER_TYPES[args.bin]
    
    send_signal(
        port=args.port,
        baudrate=args.baudrate,
        speed=args.speed,
        duration=args.duration,
        litter=litter_value
    )


if __name__ == "__main__":
    main()
