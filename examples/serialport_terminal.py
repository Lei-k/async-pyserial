if __name__ == "__main__":
    from async_pyserial import SerialPortOptions, SerialPort, SerialPortEvent, set_async_worker

    import argparse
    
    from threading import Thread
    
    parser = argparse.ArgumentParser(description='Async Serial Port Communication Tool')
    parser.add_argument('port', type=str, help='The COM port to use (e.g., COM3 or /dev/ttyUSB0)')
    parser.add_argument('--async-worker', type=str, default="none", choices=["none", "gevent", "eventlet", "asyncio"], help='async worker (default: none)')
    parser.add_argument('--read-mode', type=str, default="from-event", choices=["from-event", "from-read"], help='read mode (default: from-event)')
    parser.add_argument('--internal-read-bufsize', type=int, default=0, help='internal read bufsize (default: 1024)')
    parser.add_argument('--baudrate', type=int, default=9600, help='Baud rate (default: 9600)')
    parser.add_argument('--bytesize', type=int, choices=[5, 6, 7, 8], default=8, help='Number of data bits (default: 8)')
    parser.add_argument('--stopbits', type=int, choices=[1, 1.5, 2], default=1, help='Number of stop bits (default: 1)')
    parser.add_argument('--parity', type=int, choices=[0, 1, 2, 3, 4], default=0, help='Parity (0: None, 1: Odd, 2: Even, 3: Mark, 4: Space, default: 0)')
    
    args = parser.parse_args()
    
    if args.async_worker == 'gevent':
        from gevent import monkey
        
        monkey.patch_all()
    elif args.async_worker == 'eventlet':
        from eventlet import monkey_patch
        
        monkey_patch()
    
    def on_receieved(data):
        
        print(len(data), data)
        
    def run_read():
        try:
            while True:
                buf = serial.read()
                
                print(f'receieved: {buf}')
        except Exception as ex:
            print(ex)
            
    async def async_run_read():
        while True:
            buf = await serial.read()
            
            print(f'receieved: {buf}')
            
    
    options = SerialPortOptions()
    options.baudrate = args.baudrate
    options.bytesize = args.bytesize
    options.stopbits = args.stopbits
    options.parity = args.parity
    
    if args.async_worker != 'none':
        set_async_worker(args.async_worker)
        
    if args.internal_read_bufsize > 0:
        options.read_bufsize = args.internal_read_bufsize

    serial = SerialPort(args.port, options)
    
    print(f'read mode: {args.read_mode}')
        
    serial.open()
    
    def run():
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
            
    async def async_run():
        async_run_read()
        
        try:
            while True:
                data_to_send = input("Enter to send data or ('exit' to quit): ")
                if data_to_send.lower() == 'exit':
                    break
                
                size = 100 * 1024

                # # 生成指定大小的字节对象
                large_bytes = bytes([42] * size)
                
                #serial.write(large_bytes, lambda err: print(f'error: {err}'))
                
                await serial.write(data_to_send.encode('utf-8'))
        except KeyboardInterrupt:
            pass
        except Exception as ex:
            print(ex)
        finally:
            serial.close()
            
    if args.async_worker == 'gevent':
        import gevent
        
        tasks = []
        
        serial.on(SerialPortEvent.ON_DATA, on_receieved)

        t = gevent.spawn(run)
        
        t.join()
        
    elif args.async_worker == 'eventlet':
        import eventlet
        
        pool = eventlet.GreenPool()
        
        tasks = []
        
        serial.on(SerialPortEvent.ON_DATA, on_receieved)
        
        t = pool.spawn(run)
        
        t.wait()
            
    elif args.async_worker == 'asyncio':
        import asyncio
        
        asyncio.run(async_run())
    else:        
        if args.read_mode == 'from-event':
            serial.on(SerialPortEvent.ON_DATA, on_receieved)
        else:
            if args.async_worker != 'asyncio':
                print('start read thread')
                read_t = Thread(target=run_read)
                
                read_t.daemon = True
                
                read_t.start()
        
        run()