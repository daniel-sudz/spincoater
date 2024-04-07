import subprocess, time, pathlib

import json5.parser
import serial, json5

# locate serial port
com_port = f'/dev/{subprocess.check_output("ls /dev | grep tty.usbmodem", shell=True, encoding="utf8").strip()}'
print(f"Located serial port at {com_port}!")

# connect to serial port
baudRate = 115200
print("WAITING for serial port to connect!")
serialPort = serial.Serial(com_port, baudRate, timeout=None)
time.sleep(2) # bug in pyserial library returning before binding: https://stackoverflow.com/a/49429639

# writes to serial port and then flushes port
def writeline_with_flush(str: str):
  serialPort.write(bytes(str + "\n", "utf8"))
  serialPort.flush()

# read a line from serial
def read_line():
    if(serialPort.in_waiting != 0):
        return serialPort.readline().decode("ascii").strip()
    else:
        return None

def main():
    spin_program = (pathlib.Path(__file__).parent / "program.json").read_text()
    spin_program = json5.loads(spin_program)
    print(spin_program)

    last_executed_time = time.time()

    def get_current_rpm():
        if(len(spin_program) == 0):
            print("DONE EXECUTING PROGRAM")
            exit(0)
        return str(spin_program[0]["RPM"])

    # send first command
    writeline_with_flush(get_current_rpm())

    while True:
        # log debug from board
        new_line = read_line()
        if(new_line):
            print(new_line)

        # check if time for next command
        current_time = time.time()
        if (current_time - last_executed_time) > spin_program[0]["HOLD_SECONDS"]:
            spin_program = spin_program[1:]
            last_executed_time = current_time
            writeline_with_flush(get_current_rpm())
        

if __name__ == "__main__":
    main()