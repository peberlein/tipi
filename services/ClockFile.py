
import time
import logging
from Pab import *


class ClockFile(object):

    @staticmethod
    def filename():
        return "CLOCK"

    def __init__(self, tipi_io):
        self.tipi_io = tipi_io
        self.mode = 0

    def handle(self, pab, devname):
        op = opcode(pab)
        if op == OPEN:
            self.open(pab, devname)
        elif op == CLOSE:
            self.close(pab, devname)
        elif op == READ:
            self.read(pab, devname)
        elif op == WRITE:
            self.write(pab, devname)
        else:
            self.tipi_io.send([EOPATTR])

    def close(self, pab, devname):
        self.tipi_io.send([SUCCESS])
        self.mode = 0

    def open(self, pab, devname):
        if dataType(pab) == DISPLAY:
            if recordLength(pab) == 0 or recordLength(pab) == 19:
                self.tipi_io.send([SUCCESS])
                self.tipi_io.send([19])
                self.mode = 19
                return
            if recordLength(pab) == 24:
                self.tipi_io.send([SUCCESS])
                self.tipi_io.send([24])
                self.mode = 24
                return
        self.tipi_io.send([EOPATTR])

    def read(self, pab, devname):
        if dataType(pab) == DISPLAY:
            fdata = self.getTime()
            self.tipi_io.send([SUCCESS])
            self.tipi_io.send(fdata)
            return
        self.tipi_io.send([EOPATTR])

    def write(self, pab, devname):
        # ignore, by dropping the data.
        self.tipi_io.send([SUCCESS])
        self.tipi_io.receive()
        self.tipi_io.send([SUCCESS])

    def getTime(self):
        if mode == 24:
            return bytearray(time.asctime())
        else:
            return bytearray('%d,%s' % ((time.localtime().tm_wday + 1 % 7) + 1, time.strftime("%m/%d/%y,%H:%M:%S", time.localtime())))
       
