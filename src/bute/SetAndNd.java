package bute;

import tw.exact.XBitSet;

class SetAndNd {
    XBitSet set;
    XBitSet nd;

    public SetAndNd(XBitSet set, XBitSet nd) {
        this.set = (XBitSet) set.clone();
        this.nd = (XBitSet) nd.clone();
    }
}
