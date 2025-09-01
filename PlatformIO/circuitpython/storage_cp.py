import board, busio
import sdcardio
import storage
from .board_config import STORAGE


def mount_sd():
    spi = busio.SPI(board.IO40, MOSI=board.IO41, MISO=board.IO38)
    cs = board.IO39
    sd = sdcardio.SDCard(spi, cs)
    vfs = storage.VfsFat(sd)
    storage.mount(vfs, "/sd")
    return "/sd"
