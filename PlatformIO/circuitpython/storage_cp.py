import board
import sdcardio
import storage
from board_config import STORAGE


def mount_sd():
    # If already mounted, reuse
    try:
        storage.getmount('/sd')
        return '/sd'
    except Exception:
        pass

    # Reuse the board's SPI bus (shared with display)
    spi = board.SPI()
    # sdcardio expects the CS as a Pin, not a DigitalInOut
    sd = sdcardio.SDCard(spi, board.IO39)
    vfs = storage.VfsFat(sd)
    storage.mount(vfs, "/sd")
    return "/sd"
