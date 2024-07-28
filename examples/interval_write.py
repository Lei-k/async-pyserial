if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description='Async Serial Port Communication Tool')
    parser.add_argument('port', type=str, help='The COM port to use (e.g., COM3 or /dev/ttyUSB0)')
    parser.add_argument('--async-worker', type=str, default="none", choices=["none", "gevent", "eventlet", "asyncio"], help='async worker (default: none)')
    parser.add_argument('--use-callback', action='store_true', help='use callback (default: False)')
    parser.add_argument('--baudrate', type=int, default=9600, help='Baud rate (default: 9600)')
    parser.add_argument('--bytesize', type=int, choices=[5, 6, 7, 8], default=8, help='Number of data bits (default: 8)')
    parser.add_argument('--stopbits', type=int, choices=[1, 1.5, 2], default=1, help='Number of stop bits (default: 1)')
    parser.add_argument('--parity', type=int, choices=[0, 1, 2, 3, 4], default=0, help='Parity (0: None, 1: Odd, 2: Even, 3: Mark, 4: Space, default: 0)')
    
    args = parser.parse_args()

    async_worker = args.async_worker

    from async_pyserial import SerialPortOptions, SerialPort, SerialPortEvent, set_async_worker

    if async_worker != "none":
        set_async_worker(async_worker)

    if async_worker == 'gevent':
        print('use gevent worker')

        from gevent import monkey

        monkey.patch_all()
    elif async_worker == 'eventlet':
        from eventlet import monkey_patch

        monkey_patch()

    import time
    
    def on_data(data):
        
        print(len(data), data)

    def run():
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
                size = 1000 * 1024

                # # 生成指定大小的字节对象
                large_bytes = bytes([42] * size)

                callback = None

                if args.use_callback:
                    print('use callback')
                    callback = lambda err: err

                start = time.time()

                print('start write')

                serial.write(large_bytes, callback)

                end = time.time()

                print('{} bytes written, consume time: {:.4f}s'.format(len(large_bytes), end-start))

                if async_worker == 'gevent':
                    import gevent

                    gevent.sleep(1)
                elif async_worker == 'eventlet':
                    import eventlet

                    eventlet.sleep(1)
                else:
                    time.sleep(1)
        except KeyboardInterrupt:
            pass
        except Exception as ex:
            print(ex)
        finally:
            serial.close()

    async def async_run():
        import asyncio

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
                size = 1000 * 1024

                # # 生成指定大小的字节对象
                large_bytes = bytes([42] * size)

                start = time.time()

                await serial.write(large_bytes)

                end = time.time()

                print('{} bytes written, consume time: {:.4f}s'.format(len(large_bytes), end-start))

                await asyncio.sleep(1)
        except KeyboardInterrupt:
            pass
        except Exception as ex:
            print(ex)
        finally:
            serial.close()

    if async_worker == 'gevent':
        import gevent

        gevent.joinall([gevent.spawn(run)])
    elif async_worker == 'eventlet':
        import eventlet

        task = eventlet.spawn(run)

        task.wait()
    elif async_worker == 'asyncio':
        import asyncio

        asyncio.run(async_run())
    else:
        run()