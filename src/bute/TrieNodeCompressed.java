package bute;

import java.util.ArrayList;
import java.util.Arrays;

import tw.exact.XBitSet;

class TrieNodeCompressed {
    private TrieNodeCompressed[] children = new TrieNodeCompressed[0];
    XBitSet subtrieIntersectionOfSSets;
    XBitSet subtrieIntersectionOfNSets;
    int[] key = new int[0];
    private XBitSet[] SSets = new XBitSet[0];

    TrieNodeCompressed(int[] key, XBitSet initialIntersectionOfNSets,
            XBitSet initialIntersectionOfSSets) {
        this.key = key;
        subtrieIntersectionOfNSets = initialIntersectionOfNSets;
        subtrieIntersectionOfSSets = initialIntersectionOfSSets;
    }

    void addSSet(XBitSet SSet) {
        SSets = Arrays.copyOf(SSets, SSets.length + 1);
        SSets[SSets.length - 1] = (XBitSet) SSet.clone();
    }

    int commonPrefixLength(int[] a, int[] b) {
        int minLen = a.length <= b.length ? a.length : b.length;
        int retval = 0;
        for (int i=0; i<minLen; i++) {
            if (a[i] != b[i]) {
                return retval;
            }
            ++retval;
        }
        return retval;
    }

    TrieNodeCompressed getOrAddChildNode(int[] key, XBitSet SSet, XBitSet NSet) {
        for (int i=0; i<children.length; i++) {
            TrieNodeCompressed child = children[i];
            int prefixLen = commonPrefixLength(child.key, key);
            if (prefixLen == child.key.length) {
                // child.key is a prefix of key
                child.subtrieIntersectionOfNSets.and(NSet);
                child.subtrieIntersectionOfSSets.and(SSet);
                return child;
            } else if (prefixLen != 0) {
                int[] prefix = Arrays.copyOf(key, prefixLen);
                TrieNodeCompressed node = new TrieNodeCompressed(prefix, (XBitSet) child.subtrieIntersectionOfNSets.clone(),
                        (XBitSet) child.subtrieIntersectionOfSSets.clone());
                node.subtrieIntersectionOfNSets.and(NSet);
                node.subtrieIntersectionOfSSets.and(SSet);
                node.children = new TrieNodeCompressed[] {child};
                child.key = Arrays.copyOfRange(child.key, prefixLen, child.key.length);
                children[i] = node;
                return node;
            }
        }
        // Node not found; add and return it
        TrieNodeCompressed node = new TrieNodeCompressed(key, (XBitSet) NSet.clone(), (XBitSet) SSet.clone());
        children = Arrays.copyOf(children, children.length + 1);
        children[children.length - 1] = node;
        return node;
    }

    void query(XBitSet querySUnionN, XBitSet queryN, int k,
            int budget, ArrayList<SetAndNd> out_list) {
        if (subtrieIntersectionOfNSets.subtract(queryN).cardinality() > k) {
            return;
        }
        if (querySUnionN.intersects(subtrieIntersectionOfSSets)) {
            return;
        }
        for (XBitSet SSet : SSets) {
            if (!querySUnionN.intersects(SSet)) {
                SetAndNd elem = new SetAndNd(SSet, subtrieIntersectionOfNSets);
                out_list.add(elem);
            }
        }
        for (TrieNodeCompressed child : children) {
            int newBudget = budget;
            for (int v : child.key) {
                if (!queryN.get(v)) {
                    --newBudget;
                }
            }
            if (newBudget >= 0) {
                child.query(querySUnionN, queryN, k, newBudget, out_list);
            }
        }
    }

    private void printLatexBitset(XBitSet bitset, String colour) {
        System.out.print("\\\\ [-1ex] \\scriptsize {\\color{" + colour + "} $");
        if (bitset.isEmpty()) {
            System.out.print("\\emptyset");
        } else {
            String comma = "";
            for (int v = bitset.nextSetBit(0); v >= 0; v = bitset.nextSetBit(v+1)) {
                System.out.print(comma + v);
                comma = " ";
            }
        }
        System.out.print("$} ");
    }

    void printLatex(int prevNodeNLength, ArrayList<Integer> currentNodeN, int featureFlags) {
        System.out.print("[{$");
        if (currentNodeN.isEmpty()) {
            System.out.print("\\emptyset");
        } else {
            for (int i=0; i<currentNodeN.size(); i++) {
            int v = currentNodeN.get(i);
                if (i == prevNodeNLength) {
                    System.out.print("\\mathbf{\\underline{");
                }
                System.out.print(v);
            }
            System.out.print("}}");
        }
        System.out.print("$ ");

        if (0 != (featureFlags & 1)) {
            printLatexBitset(subtrieIntersectionOfNSets, "black!50");
        }

        if (0 != (featureFlags & 2)) {
            printLatexBitset(subtrieIntersectionOfSSets, "blue");
        }

        if (0 != (featureFlags & 4)) {
            for (XBitSet SSet : SSets) {
                printLatexBitset(SSet, "blue!50");
            }
        }
        System.out.print("},align=center");
        if (SSets.length > 0) {
            System.out.print(",line width=.7mm");
        }
        for (TrieNodeCompressed child : children) {
            int len = currentNodeN.size();
            for (int v : child.key) {
                currentNodeN.add(v);
            }
            child.printLatex(len, currentNodeN, featureFlags);
            for (int v : child.key) {
                currentNodeN.remove(currentNodeN.size() - 1);
            }
        }
        System.out.print("]");
    }
}
