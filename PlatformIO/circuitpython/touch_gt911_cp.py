import board, busio
from micropython import const

GT911_ADDR = const(0x5D)
REG_STATUS = const(0x814E)
REG_FIRST_POINT = const(0x8150)

class TouchGT911:
    def __init__(self):
        self.i2c = busio.I2C(board.IO8, board.IO18)  # scl, sda
        while not self.i2c.try_lock():
            pass
        self.i2c.unlock()
        self.last = (0, 0, False)

    def _mem_read(self, reg, n):
        buf = bytearray(n)
        regb = bytes([(reg >> 8) & 0xFF, reg & 0xFF])
        with self.i2c as i2c:
            i2c.writeto(GT911_ADDR, regb, stop=False)
            i2c.readfrom_into(GT911_ADDR, buf)
        return buf

    def _mem_write(self, reg, val):
        if isinstance(val, int):
            val = bytes([val])
        regb = bytes([(reg >> 8) & 0xFF, reg & 0xFF])
        with self.i2c as i2c:
            i2c.writeto(GT911_ADDR, regb + val)

    def read(self):
        try:
            status = self._mem_read(REG_STATUS, 1)[0]
        except Exception:
            return (0, 0, False)
        touched = status & 0x80
        points = status & 0x0F
        if not touched or points == 0:
            return (self.last[0], self.last[1], False)
        try:
            data = self._mem_read(REG_FIRST_POINT, 8)
        except Exception:
            return (self.last[0], self.last[1], False)
        x = (data[1] << 8) | data[0]
        y = (data[3] << 8) | data[2]
        self._mem_write(REG_STATUS, 0)
        self.last = (x, y, True)
        return self.last
