import glob
import serial
import subprocess


def lstPort():
    for port in glob.glob('/dev/tty[A-Za-z]*'):
        try:
            s = serial.Serial(port)
            s.close()
            return port
        except (OSError, serial.SerialException):
            pass
    print('Connecting Error ...')


def portNew():
    strDevice = str(subprocess.check_output(
        'python3 -m serial.tools.list_ports_linux', shell=True))
    return strDevice[2:strDevice.find(':')]
