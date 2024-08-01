from async_pyserial.common import SerialPortOptions, SerialPortEvent, SerialPortBase, SerialPortError

from typing import Callable

from concurrent.futures import Future

from async_pyserial import backend

from threading import RLock

class SerialPort(SerialPortBase):
    def __init__(self, portName: str, options: SerialPortOptions) -> None:
        
        super().__init__(portName, options)

        from async_pyserial import async_pyserial_core
        
        self._internal = async_pyserial_core.SerialPort(portName, self.internal_options)

        self._is_open = False
        
        self._read_bufsize = options.read_bufsize
        
        self._read_buf = b''
        
        self._rlock = RLock()
            
        def on_receieved(data):
            if self._read_bufsize > 0:
                with self._rlock:
                    actual_size = min(len(data), max(self._read_bufsize - len(self._read_buf), 0))
                    
                    if actual_size > 0:
                        self._read_buf += data[:actual_size]
            
            self.emit(SerialPortEvent.ON_DATA, data)
        
        self._internal.set_data_callback(on_receieved)

    def _calculate_stt(self, data_size):
        """
        Calculate the Serial Transmission Time (STT).
        
        Parameters:
        data_size (int): The size of the data packet in bytes.
        
        Returns:
        float: The estimated transmission time in seconds.
        """
        # Calculate the transmission time, considering start bit, data bits, and stop bit
        stt = (data_size * 10) / self.options.baudrate
        return stt
    
    def read(self, bufsize: int = 512, callback: Callable | None = None):
        """
        Read data from the serial port. If a callback is provided, the read will be asynchronous and 
        the callback will be called with the read data. Otherwise, the read will be synchronous or asynchronous
        depending on whether an async_worker is set.

        Note:
            It is recommended to set SerialPortEvent.ON_DATA to receive data.
            If you use read(), ensure that the read_bufsize in options is set appropriately
            to prevent data loss. Setting a larger read_bufsize ensures that enough data
            is buffered internally to handle the read operation.

            By default, read_bufsize is 0, meaning only data received after the read call,
            up to the specified bufsize, will be returned.

            This read method supports different async workers:
            - gevent
            - eventlet
            - asyncio

            If async_worker is set using async_pyserial.set_async_worker(`async-worker`), 
            the read method will use asynchronous processing.
        """
        if backend.async_worker == 'gevent':
            return self._gevent_read(bufsize)
        elif backend.async_worker == 'eventlet':
            return self._eventlet_read(bufsize)
        elif backend.async_worker == 'asyncio':
            return self._asyncio_read(bufsize)
        elif callback is not None:
            self._callback_read(bufsize, callback)
        else:
            return self._sync_read(bufsize)
        
    def _callback_read(self, bufsize: int, callback: Callable):
        if self._read_bufsize <= 0:            
            def on_receieved(data):
                self.off(SerialPortEvent.ON_DATA, on_receieved)
                
                actual_size = min(len(data), bufsize)
                
                buf = data[:actual_size]
                
                callback(buf)
                
            self.on(SerialPortEvent.ON_DATA, on_receieved)
            
            return
        
        if self._read_buf != b'':
            # some data have in internal read buf
            # return buf with max bufsize directly
            with self._rlock:
                actual_size = min(len(self._read_buf), bufsize)
                
                buf = self._read_buf[:actual_size]
                
                self._read_buf = self._read_buf[actual_size:]
                
            callback(buf)
                
            return
                
        def on_receieved(_: bytes):
            self.off(SerialPortEvent.ON_DATA, on_receieved)
            
            # read from _read_buf
            actual_size = min(len(self._read_buf), bufsize)

            print(f'size: {len(self._read_buf)}')
                
            buf = self._read_buf[:actual_size]
            
            self._read_buf = self._read_buf[actual_size:]
            
            callback(buf)
            
        self.on(SerialPortEvent.ON_DATA, on_receieved)
        
    def _sync_read(self, bufsize: int):
        future = Future()
        
        def on_receieved(data):
            future.set_result(data)
        
        self._callback_read(bufsize, on_receieved)
        
        return future.result()
        
    def _gevent_read(self, bufsize: int):
        
        import gevent
        from gevent.event import AsyncResult
        
        ar = AsyncResult()
        
        def on_receieved(data):
            ar.set(data)
            
        self._callback_read(bufsize, on_receieved)
            
        # calc stt for read bufsize
        stt = self._calculate_stt(bufsize)
        wt = stt / 20.0
        
        wt = min(wt, 0.05)  # max wait time is 0.05s

        while not ar.ready():
            gevent.sleep(wt)
    
        
        buf = ar.get()
        
        return buf
    
    def _eventlet_read(self, bufsize: int):
        
        import eventlet
        from eventlet.event import Event

        evt = Event()
        
        def on_receieved(data):
            evt.send(data)
            
        self._callback_read(bufsize, on_receieved)
            
        # calc stt for read bufsize
        stt = self._calculate_stt(bufsize)
        wt = stt / 20.0
        
        wt = min(wt, 0.05)  # max wait time is 0.05s
            
        while not evt.ready():
            eventlet.sleep(wt)
            
        buf = evt.wait()
        
        return buf
    
    def _asyncio_read(self, bufsize: int):
        import asyncio

        loop = backend.async_loop

        if loop is None:
            loop = asyncio.get_running_loop()

        future = loop.create_future()

        def on_receieved(data):
            loop.call_soon_threadsafe(future.set_result, data)
        
        self._callback_read(bufsize, on_receieved)
        
        return future
        
    def write(self, data: bytes, callback: Callable | None = None):
        """
        Write data to the serial port. If a callback is provided, the write will be asynchronous and 
        the callback will be called with the result. Otherwise, the write will be synchronous or asynchronous
        depending on whether an async_worker is set.

        Note:
            This write method supports different async workers:
            - gevent
            - eventlet
            - asyncio

            If async_worker is set using async_pyserial.set_async_worker(`async-worker`), 
            the write method will use asynchronous processing.

        Args:
            data (bytes): The data to be written to the serial port.
            callback (Callable, optional): The callback to be called with the result of the write operation.

        Raises:
            SerialPortError: If the write operation fails.
        """
        if backend.async_worker == 'gevent':
            import gevent
            from gevent.event import AsyncResult

            ar = AsyncResult()

            def cb(err):
                ar.set(err)

            self._internal.write(data, cb)

            stt = self._calculate_stt(len(data))
            wt = stt / 20.0

            if wt > 0.05:
                # max wait time is 0.05s
                wt = 0.05

            while not ar.ready():
                # maybe can use ar.wait(timeout=0.01)
                # but it look like unsable
                # ar.wait(timeout=0.01) test history
                # Invalid switch into <Greenlet at 0x103661ee0: run>: 
                # got <gevent._gevent_cevent.AsyncResult object at 0x103f6a570> 
                # (expected <gevent._gevent_c_waiter.Waiter object at 0x103ffddf0>; 
                # waiting on <Hub '' at 0x103fbb560 select default pending=0 ref=1 thread_ident=0x1e57d2080> 
                # with <timer at 0x103bd6d40 native=0x103bd6d80 active callback=<bound method Waiter.switch 
                # of <gevent._gevent_c_waiter.Waiter object at 0x103ffddf0>> args=(<gevent._gevent_c_waiter.Waiter 
                # object at 0x103ffddf0>,)>)
                gevent.sleep(wt)

            err = ar.get()

            if err != 0:
                raise SerialPortError('write failure')
        elif backend.async_worker == 'eventlet':
            import eventlet
            from eventlet.event import Event

            evt = Event()

            def cb(err):
                
                evt.send(err)

            self._internal.write(data, cb)

            stt = self._calculate_stt(len(data))
            wt = stt / 20.0

            if wt > 0.05:
                # max wait time is 0.05s
                wt = 0.05

            while not evt.ready():
                eventlet.sleep(wt)

            err = evt.wait()

            if err != 0:
                raise SerialPortError('write failure')
        elif backend.async_worker == 'asyncio':
            import asyncio

            loop = backend.async_loop

            if loop is None:
                loop = asyncio.get_running_loop()

            future = loop.create_future()

            def cb(err):
                if err == 0:
                    loop.call_soon_threadsafe(future.set_result, None)
                else:
                    loop.call_soon_threadsafe(future.set_exception, SerialPortError('write failure'))
            
            self._internal.write(data, cb)

            return future
        elif callback is not None:
            self._internal.write(data, callback)
        else:
            future = Future()

            def cb(err):
                future.set_result(err)

            self._internal.write(data, cb)

            err = future.result()

            if err != 0:
                raise SerialPortError('write failure')
        
    def open(self):
        self._internal.open()
        self._is_open = True
        
    def close(self):
        self._internal.close()
        self._is_open = False

    def is_open(self):
        return self._is_open