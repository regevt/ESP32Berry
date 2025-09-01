import board, audiobusio, audiocore

class WavPlayer:
    def __init__(self):
        self.i2s = audiobusio.I2SOut(bit_clock=board.IO7, word_select=board.IO5, data=board.IO6)

    def play(self, path):
        with open(path, 'rb') as f:
            wav = audiocore.WaveFile(f)
            self.i2s.play(wav)
            while self.i2s.playing:
                pass
            self.i2s.stop()
