package bute;

import java.util.ArrayList;

import tw.exact.XBitSet;

class ListOfSetAndNd implements TrieDataStructure {
    ArrayList<SetAndNd> list = new ArrayList<>();

    public void put(SetAndNd setAndNd) {
        list.add(setAndNd);
    }

    public ArrayList<SetAndNd> query(XBitSet querySUnionN, XBitSet neighbours) {
        return list;
    }
}
