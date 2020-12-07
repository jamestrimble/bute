package bute;

import java.util.ArrayList;
import java.util.Arrays;

import tw.exact.XBitSet;

class NewTrieCompressed implements TrieDataStructure, LatexPrintable {
    private int target;
    private TrieNodeCompressed root;
    private int size;

    NewTrieCompressed(int n, int target) {
        this.target = target;
        XBitSet initialIntersectionOfNSets = new XBitSet();
        initialIntersectionOfNSets.set(0, n);
        XBitSet initialIntersectionOfSSets = new XBitSet();
        initialIntersectionOfSSets.set(0, n);
        root = new TrieNodeCompressed(new int[0], initialIntersectionOfNSets,
                initialIntersectionOfSSets);
    }

    public void put(SetAndNd setAndNd) {
        XBitSet SSet = setAndNd.set;
        XBitSet NSet = setAndNd.nd;
        root.subtrieIntersectionOfNSets.and(NSet);
        root.subtrieIntersectionOfSSets.and(SSet);
        TrieNodeCompressed node = root;

        int[] key = NSet.toArray();

        while (key.length != 0) {
            node = node.getOrAddChildNode(key, SSet, NSet);
            key = Arrays.copyOfRange(key, node.key.length, key.length);
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
        root.printLatex(0, new ArrayList<Integer>(), featureFlags);
        System.out.println();
        System.out.println("\\end{forest}");
        System.out.println("\\end{document}");
    }
}
