from async_pyserial.common import SerialPortOptions, SerialPortEvent, SerialPortBase, SerialPortError

from typing import Callable

from concurrent.futures import Future

from async_pyserial import backend

import threading

class SerialPort(SerialPortBase):
    def __init__(self, portName: str, options: SerialPortOptions) -> None:
        
        super().__init__(portName, options)

        from async_pyserial import async_pyserial_core
        
        self.internal = async_pyserial_core.SerialPort(portName, self.internal_options)
            
        def on_data(data):
            self.emit(SerialPortEvent.ON_DATA, data)
        
        self.internal.set_data_callback(on_data)
    
    def read(self, bufsize: int, callback: Callable | None = None):
        if callback is None:
            pass
        else:
            pass

    def __calculate_stt(self, data_size):
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
        
    def write(self, data: bytes, callback: Callable | None = None):
        if backend.async_worker == 'gevent':
            import gevent
            from gevent.event import AsyncResult

            ar = AsyncResult()

            def cb(err):
                ar.set(err)

            self.internal.write(data, cb)

            stt = self.__calculate_stt(len(data))
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

            self.internal.write(data, cb)

            stt = self.__calculate_stt(len(data))
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
            
            self.internal.write(data, cb)

            return future
        elif callback is not None:
            self.internal.write(data, callback)
        else:
            future = Future()

            def cb(err):
                future.set_result(err)

            self.internal.write(data, cb)

            err = future.result()

            if err != 0:
                raise SerialPortError('write failure')
        
    def open(self):
        self.internal.open()
        
    def close(self):
        self.internal.close()