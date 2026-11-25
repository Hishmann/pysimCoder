from supsisim.RCPblk import RCPblk, RcpParam
from numpy import size

def can_recvBlk(pout: list[int], params: RcpParam) -> RCPblk:

    if size(pout) != size(params[2].value):
        raise ValueError(
            "Number of inputs (%i) should match the data count (%i) in array."
            % (size(pout), size(params[2].value))
        )
    if size(params[2].value) != size(params[3].value):
        raise ValueError(
            "Size of Bits array (%i) should match size of data array (%i)."
            % (size(params[2].value), size(params[3].value))
        )

    blk = RCPblk('can_recv', [], pout, [0,0], 0, params)
    return blk
