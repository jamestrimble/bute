package bute;

import java.util.ArrayList;

import tw.exact.XBitSet;

interface TrieDataStructure {
    void put(SetAndNd setAndNd);

    ArrayList<SetAndNd> query(XBitSet querySUnionN, XBitSet neighbours);
}
