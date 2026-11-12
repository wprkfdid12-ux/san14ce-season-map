import struct, zlib
def parse(cur):
    """IDRK container -> (hdr_len, chunks(list of compressed bytes), end_pos)"""
    hdr=int.from_bytes(cur[8:12],'little')-int.from_bytes(cur[16:20],'little')
    p=hdr; cs=[]
    while True:
        L,=struct.unpack_from('<I',cur,p)
        if L==0: break
        cs.append(bytes(cur[p+4:p+4+L])); p+=4+L
    return hdr,cs,p
def raw(cur):
    h,cs,e=parse(cur); return b''.join(zlib.decompress(c) for c in cs)
