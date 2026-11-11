import zlib,struct
def zexact(data, size, level=9):
    """zlib stream of data with exactly `size` bytes (None if impossible: too small)."""
    for n in range(0, 64):
        c=zlib.compressobj(level, zlib.DEFLATED, -15)
        head=c.compress(data[:len(data)-n]) + c.flush(zlib.Z_SYNC_FLUSH)
        base=2+len(head)+5+n+4
        rest=size-base
        if rest<0: return None
        if rest%5==0:
            k=rest//5
            tail=data[len(data)-n:]
            s=b'\x78\xda'+head+b'\x00\x00\x00\xff\xff'*k+b'\x01'+struct.pack('<HH',n,n^0xffff)+tail+struct.pack('>I',zlib.adler32(data)&0xffffffff)
            assert len(s)==size and zlib.decompress(s)==data
            return s
    return None
def zmin(data, level=9):
    return zlib.compress(data, level)
