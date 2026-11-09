from supsisim.RCPblk import RCPblk, RcpParam
from numpy import size

def can_sendBlk(pin: list[int], params: RcpParam) -> RCPblk:
    blk = RCPblk('can_send', pin, [], [0,0], 0, params)
    return blk
