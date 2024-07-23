from async_pyserial import EventEmitter

def test_event_emitter():
    emitter = EventEmitter()

    expected = b'Hello!'

    def on_data(d):
        assert d == expected

    emitter.on('data', on_data)

    emitter.emit('data', expected)