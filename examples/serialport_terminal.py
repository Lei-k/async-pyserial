if __name__ == "__main__":
    from async_pyserial import SerialPortOptions, SerialPort, SerialPortEvent

    import argparse
    
    parser = argparse.ArgumentParser(description='Async Serial Port Communication Tool')
    parser.add_argument('port', type=str, help='The COM port to use (e.g., COM3 or /dev/ttyUSB0)')
    parser.add_argument('--baudrate', type=int, default=9600, help='Baud rate (default: 9600)')
    parser.add_argument('--bytesize', type=int, choices=[5, 6, 7, 8], default=8, help='Number of data bits (default: 8)')
    parser.add_argument('--stopbits', type=int, choices=[1, 1.5, 2], default=1, help='Number of stop bits (default: 1)')
    parser.add_argument('--parity', type=int, choices=[0, 1, 2, 3, 4], default=0, help='Parity (0: None, 1: Odd, 2: Even, 3: Mark, 4: Space, default: 0)')
    
    args = parser.parse_args()
    
    def on_data(data):
        
        print(len(data), data)
    
    options = SerialPortOptions()
    options.baudrate = args.baudrate
    options.bytesize = args.bytesize
    options.stopbits = args.stopbits
    options.parity = args.parity

    serial = SerialPort(args.port, options)
    serial.on(SerialPortEvent.ON_DATA, on_data)
    serial.open()
    
    try:
        while True:
            data_to_send = input("Enter to send data or ('exit' to quit): ")
            if data_to_send.lower() == 'exit':
                break
            
            size = 100 * 1024

            # # 生成指定大小的字节对象
            large_bytes = bytes([42] * size)
            
            #serial.write(large_bytes, lambda err: print(f'error: {err}'))
            
            serial.write(data_to_send.encode('utf-8'))
    except KeyboardInterrupt:
        pass
    except Exception as ex:
        print(ex)
    finally:
        serial.close()