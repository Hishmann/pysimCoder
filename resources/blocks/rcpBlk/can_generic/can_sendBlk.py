from supsisim.RCPblk import RCPblk, RcpParam
from numpy import size

def can_sendBlk(pin: list[int], params: RcpParam) -> RCPblk:

    if size(pin) != size(params[2].value):
        raise ValueError(
            "Number of inputs (%i) should match the data count (%i) in array."
            % (size(pin), size(params[2].value))
        )
    if size(params[2].value) != size(params[3].value):
        raise ValueError(
            "Size of Bits array (%i) should match size of data array (%i)."
            % (size(params[2].value), size(params[3].value))
        )

    blk = RCPblk('can_send', pin, [], [0,0], 0, params)
    return blk
