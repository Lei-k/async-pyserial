import pytest

import async_pyserial

@pytest.fixture(scope="module")
def virtual_platform():
    import sys

    v = 'virtual'

    # mock current `sys.platform` to virtual
    tmp = sys.platform
    sys.platform = v

    yield v

    # restore
    sys.platform = tmp

    import importlib

    importlib.reload(async_pyserial)

def test_platform(virtual_platform):
    _ = virtual_platform

    import importlib

    with pytest.raises(Exception):
        # should raise Exception when reload async_pyserial
        importlib.reload(async_pyserial)