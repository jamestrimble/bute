package bute;


class SetAndNd {
    FastBitSet set;
    FastBitSet nd;

    public SetAndNd(FastBitSet set, FastBitSet nd) {
        this.set = new FastBitSet(set);
        this.nd = new FastBitSet(nd);
    }
}
