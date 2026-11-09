from supsisim.RCPblk import RCPblk, RcpParam
from numpy import size

def can_recvBlk(pout: list[int], params: RcpParam) -> RCPblk:
    blk = RCPblk('can_recv', [], pout, [0,0], 0, params)
    return blk
