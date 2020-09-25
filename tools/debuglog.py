import serial
import argparse

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-p', '--port',
                        default=None,
                        type=str,
                        help='Port name open to get debug message.')
    args = parser.parse_args()

    # linux
    # ser = serial.Serial('/dev/ttyS0', 9600, timeout=0.2)
    # windows
    ser = serial.Serial(args.port, 9600, timeout=0.2)
    # ser = serial.Serial('COM3', 9600, timeout=0.2)


    # print debug log message
    while True:
        recv = ser.readline()
        print(str(recv))


if __name__ == '__main__':
    main()

