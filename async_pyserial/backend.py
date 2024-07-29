async_worker = 'none'

async_loop = None

def set_async_worker(w: str, loop = None):
    global async_worker, async_loop

    if w == 'gevent':
        try:
            import gevent
        except Exception:
            raise ModuleNotFoundError('can\'t set async worker to gevent when not installed')
    elif w == 'eventlet':
        try:
            import eventlet
        except Exception:
            raise ModuleNotFoundError('can\'t set async worker to eventlet when not installed')

    async_worker = w
    async_loop = loop