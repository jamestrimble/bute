package bute;

import java.util.ArrayList;
import java.util.Arrays;

import tw.exact.XBitSet;

class NewTrie implements TrieDataStructure, LatexPrintable {
    private int target;
    private TrieNode root;
    private int size;

    public NewTrie(int n, int target) {
        this.target = target;
        XBitSet initialIntersectionOfNSets = new XBitSet();
        initialIntersectionOfNSets.set(0, n);
        XBitSet initialIntersectionOfSSets = new XBitSet();
        initialIntersectionOfSSets.set(0, n);
        root = new TrieNode(-1, initialIntersectionOfNSets,
                initialIntersectionOfSSets);
    }

    public void put(SetAndNd setAndNd) {
        XBitSet SSet = setAndNd.set;
        XBitSet NSet = setAndNd.nd;
        root.subtrieIntersectionOfNSets.and(NSet);
        root.subtrieIntersectionOfSSets.and(SSet);
        TrieNode node = root;
        // iterate over elements of NSet
        for (int i = NSet.nextSetBit(0); i >= 0; i = NSet.nextSetBit(i+1)) {
            node = node.getOrAddChildNode(i, SSet, NSet);
        }
        node.addSSet(SSet);
        ++size;
    }

    // for debugging
    void showList(ArrayList<XBitSet> bitsets) {
        for (XBitSet bs : bitsets) {
            System.out.println(bs);
        }
    }

    public ArrayList<SetAndNd> query(XBitSet querySUnionN, XBitSet neighbours) {
        ArrayList<SetAndNd> list = new ArrayList<>();
        int k = target - neighbours.cardinality();
        if (k >= 0) {
            root.query(querySUnionN, neighbours, k, k, list);
        }
        return list;
    }

    public void printLatex(int featureFlags) {
        System.out.println("\\documentclass{standalone}");
        System.out.println("\\usepackage{forest}");
        System.out.println("\\forestset{  default preamble={  for tree={draw,rounded corners}  }}");
        System.out.println("\\begin{document}");
        System.out.println("\\begin{forest}");
        root.printLatex(new ArrayList<Integer>(), featureFlags);
        System.out.println();
        System.out.println("\\end{forest}");
        System.out.println("\\end{document}");
    }
}
