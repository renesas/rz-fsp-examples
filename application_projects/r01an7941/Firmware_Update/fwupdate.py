########################################################################################################################
# DISCLAIMER
# This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
# other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
# applicable laws, including copyright laws.
# THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
# THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
# EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
# SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
# SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
# Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
# this software. By using this software, you agree to the additional terms and conditions found by accessing the
# following link:
# http://www.renesas.com/disclaimer
#
# Copyright (C) 2022-2025 Renesas Electronics Corporation. All rights reserved.
########################################################################################################################
# fwupdate.py : Tool Version 3.10
import sys
import os
import argparse
import threading
import binascii
import socket

# Message print
RESULT_MSG = print
ERRMSG     = print

# Error message
ERRMSG_OPEN_FAIL = __file__ + ' : error: No such file : %s'

# Command code
CMD_START_UPDATE    = b'\x11'
CMD_FIRMWARE_UPDATE = b'\x12'
CMD_ACK             = b'\x81'
CMD_NACK            = b'\x82'

# Payload size
SIZE_START_UPDATE = 0

# ACK result message
ACK_MSG = ' Update success.'

# NACK result message
NACK_MSG = {
    1 : ' Update failure. The update file size exceeds the device receiving limit.',
    2 : ' Update failure. Failed to verify update file.',
    3 : ' Update failure. User application write failure.',
    4 : ' Update failure. Parameter for user application write failure.',
    5 : ' Update failure. Key injection failure.',
    6 : ' Update failure. Failed to switch flash management area.',
    7 : ' Update failure. User application version update failure.'
}

# UDP timeout value (second)
UDP_TIMEOUT = 10

# Endian string
ENDIAN_BIG    = 'big'
ENDIAN_LITTLE = 'little'

# Packet header parameter
DUMMY_AREA_VAL = b'\x00\x00\x00'

# Size
WORD_BYTES = 4
BUFSIZE    = 1024

class Udp:
    """UDP communication processing class.

    Send and receive by UDP communication.
    """

    def __init__(self):
        self._sock = 0

    def __del__(self):
        self.udp_close()

    def udp_open(self, port, timeout):
        """UDP open.

        Create a socket and assign the specified port number.
        Set socket timeout value.

        Args:
            port (int): UDP port number
            timeout (int): Timeout value

        Returns:
            None

        """
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self._sock.bind(('', port))
        self._sock.settimeout(timeout)

        return

    def udp_close(self):
        """UDP close.

        Close the socket.

        Args:
            None

        Returns:
            None

        """
        if self._sock != 0:
            self._sock.close()
            self._sock = 0

        return

    def udp_recv(self):
        """UDP receive.

        Receive data by UDP communication.

        Args:
            None

        Returns:
            Received data (bytes)
            Source IP address (str)

        """
        msg = ''
        msg, address = self._sock.recvfrom(BUFSIZE)

        return (msg, address[0])

    def udp_send(self, address, port, msg):
        """UDP send.

        Send data by UDP communication.

        Args:
            address (str): IP address
            port (int): UDP port number
            msg (bytes): Data to send

        Returns:
            None

        """
        self._sock.sendto(msg, (address, port))

        return

class Tcp:
    """TCP communication processing class.

    Send and receive by TCP communication.
    """

    def __init__(self):
        self._sock = 0

    def __del__(self):
        self.tcp_close()

    def tcp_open(self, address, port):
        """TCP open.

        Create a socket and connect to the specified IP address and port.

        Args:
            address (str): IP address
            port (int): TCP port number

        Returns:
            None

        """
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect((address, port))

        return

    def tcp_close(self):
        """TCP close.

        Close the socket.

        Args:
            None

        Returns:
            None

        """
        if self._sock != 0:
            self._sock.close()
            self._sock = 0

        return

    def tcp_recv(self):
        """TCP receive.

        Receive data by TCP communication.

        Args:
            None

        Returns:
            Received data (bytes)

        """
        msg = ''
        msg = self._sock.recv(BUFSIZE)

        return msg

    def tcp_send(self, msg):
        """TCP send.

        Send data by TCP communication.

        Args:
            msg (bytes): Data to send

        Returns:
            None

        """
        self._sock.sendall(msg)

        return

def main():
    """Main processing.

    Parsing command line arguments.
    Send update start notification.
    
    Args:
        None
        
    Returns:
        None

    """
    # Command line argument perse.
    parser = set_cmdline_argparser()
    
    # Parse command line arguments.
    args = parser.parse_args()
    threads = []

    # Open input file and read data.
    file_check = read_bin_file(args.i)
    if file_check is None:
        exit(1)

    try:
        udp = Udp()
        udp.udp_open(args.udp_port, UDP_TIMEOUT)

        # If the IP address is not specified in the command options,
        # send a firmware update start notification by broadcast.
        if args.ip_address is None:
            args.ip_address = '255.255.255.255'

        # Send a firmware update start notification. Create START_UPDATE message.
        # msg = code 1 byte(0x11 fixed value) + dummy data 3 byte(0x000000 fixed value) + inputfile size 4 byte(hex)
        msg = CMD_START_UPDATE + DUMMY_AREA_VAL + SIZE_START_UPDATE.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
        udp.udp_send(args.ip_address, args.udp_port, msg)

        while True:
            # Receive data.
            rcv_msg, dest_ip_address = udp.udp_recv()

            # Check if ACK is received.
            if rcv_msg[0] == CMD_ACK[0]:
                # Start "send firmware data" thread.
                t = threading.Thread(target=send_firmware_data, args=([dest_ip_address, args.tcp_port, args.i]))
                threads.append(t)
                t.start()

    except Exception:
        pass

    finally:
        # Wait for the "send firmware data" thread to finish.
        for thread in threads:
            thread.join()

        if 'udp' in locals():
            udp.udp_close()

    exit(0)

def send_firmware_data(ip_address, tcp_port, input_file):
    """Send firmware data.

    Send the update file to devices.
    
    Args:
        ip_address (str): IP address
        tcp_port   (int): TCP port number
        input_file (str): Update file path
        
    Returns:
        None

    """
    rcv_msg = ''

    try:
        tcp = Tcp()
        tcp.tcp_open(ip_address, tcp_port)

        # Open input file and read data.
        update_data = read_bin_file(input_file)

        # Create send packet.
        # header = code 1 byte(0x12 fixed value) + dummy data 3 byte(0x000000 fixed value) + inputfile size 4 byte(hex)
        header = CMD_FIRMWARE_UPDATE + DUMMY_AREA_VAL + os.path.getsize(input_file).to_bytes(WORD_BYTES, ENDIAN_LITTLE)

        # Send message = header 8 byte + inputfile data
        snd_msg = header + update_data

        # Send data.
        tcp.tcp_send(snd_msg)

        # Receive data.
        rcv_msg = tcp.tcp_recv()

        # Check response data.
        if rcv_msg[0] == CMD_ACK[0]:
            RESULT_MSG(ip_address + ACK_MSG)

        elif rcv_msg[0] == CMD_NACK[0]:
            err_code = rcv_msg[8]
            RESULT_MSG(ip_address + NACK_MSG[err_code])

        else:
            pass

    except Exception:
        pass

    finally:
        if 'tcp' in locals():
            tcp.tcp_close()

    return

def read_bin_file(file_path):
    """Read binary data from file.
    
    Reads data from the file in binary format. 
    
    Args:
        file_path (str) - Binary file path
    
    Returns:
        Bytes read from file (bytes)
    
    """
    try:
        # Open input file and read data.
        with open(file_path, mode='rb') as f:
            bin = f.read()
    except:
        ERRMSG(str.format(ERRMSG_OPEN_FAIL % (file_path)))
        bin = None
    return bin

def set_cmdline_argparser():
    """Command line argument parsing settings.
    
    Set valid commands and options on the command line.
    
    Args:
        None
    
    Returns:
        parser - ArgumentParser object
    """

    # Command option help message
    MSG_HELP_TOOL     = 'Tools to send an update file to devices'
    MSG_HELP_IP_ADDR  = 'IP address to be firmware updated'
    MSG_HELP_UDP_PORT = 'UDP port number of devices to be firmware updated. UDP reception port number'
    MSG_HELP_TCP_PORT = 'TCP port number of devices to be firmware updated'
    MSG_HELP_I        = 'Update file to be sent to devices'
    
    METAVER_IP   = '<IP address>'
    METAVER_PORT = '<Port>'
    METAVER_FILE = '<File>'

    parser = argparse.ArgumentParser(description=MSG_HELP_TOOL)

    # Add arguments to receive.
    parser.add_argument('--ip_address', help=MSG_HELP_IP_ADDR, metavar=METAVER_IP)
    parser.add_argument('--udp_port', help=MSG_HELP_UDP_PORT, metavar=METAVER_PORT, required=True, type=int)
    parser.add_argument('--tcp_port', help=MSG_HELP_TCP_PORT, metavar=METAVER_PORT, required=True, type=int)
    parser.add_argument('-i', help=MSG_HELP_I, metavar=METAVER_FILE, required=True)
    
    return parser

if __name__ == '__main__':
    main()
