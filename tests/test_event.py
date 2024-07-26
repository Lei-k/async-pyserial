from async_pyserial import EventEmitter

def test_emit_event():
    emitter = EventEmitter()

    expected = b'Hello!'

    def on_data(d):
        assert d == expected

    emitter.on('data', on_data)

    emitter.emit('data', expected)

def test_decorator():
    emitter = EventEmitter()

    expected = b'Hello!'

    @emitter.on('data')
    def on_data(d):
        assert d == expected

    emitter.emit('data', expected)

def test_remove_listener():
    emitter = EventEmitter()

    def on_data():
        # this func should not be call
        assert False

    emitter.on('data', on_data)

    emitter.remove_listener('data', on_data)

    emitter.emit('data')

def test_off():
    emitter = EventEmitter()

    def on_data():
        # this func should not be call
        assert False

    emitter.on('data', on_data)

    emitter.off('data', on_data)

    emitter.emit('data')

def test_remove_all_listener():
    emitter = EventEmitter()

    def on_data():
        # this func should not be call
        assert False

    emitter.on('data', on_data)

    emitter.remove_all_listeners('data')

    emitter.emit('data')