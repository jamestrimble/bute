package bute;

import java.util.ArrayList;
import java.util.Arrays;


class TrieNodeCompressed {
    private TrieNodeCompressed[] children = new TrieNodeCompressed[0];
    FastBitSet subtrieIntersectionOfSSets;
    FastBitSet subtrieIntersectionOfNSets;
    int[] key;
    private FastBitSet[] SSets = new FastBitSet[0];

    TrieNodeCompressed(int[] key, FastBitSet initialIntersectionOfNSets,
            FastBitSet initialIntersectionOfSSets) {
        this.key = key;
        subtrieIntersectionOfNSets = initialIntersectionOfNSets;
        subtrieIntersectionOfSSets = initialIntersectionOfSSets;
    }

    void addSSet(FastBitSet SSet) {
        SSets = Arrays.copyOf(SSets, SSets.length + 1);
        SSets[SSets.length - 1] = new FastBitSet(SSet);
    }

    int commonPrefixLength(int[] a, int[] b) {
        int minLen = Math.min(a.length, b.length);
        int retval = 0;
        for (int i=0; i<minLen; i++) {
            if (a[i] != b[i]) {
                return retval;
            }
            ++retval;
        }
        return retval;
    }

    TrieNodeCompressed getOrAddChildNode(int[] key, FastBitSet SSet, FastBitSet NSet) {
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
                TrieNodeCompressed node = new TrieNodeCompressed(prefix, new FastBitSet(child.subtrieIntersectionOfNSets),
                        new FastBitSet(child.subtrieIntersectionOfSSets));
                node.subtrieIntersectionOfNSets.and(NSet);
                node.subtrieIntersectionOfSSets.and(SSet);
                node.children = new TrieNodeCompressed[] {child};
                child.key = Arrays.copyOfRange(child.key, prefixLen, child.key.length);
                children[i] = node;
                return node;
            }
        }
        // Node not found; add and return it
        TrieNodeCompressed node = new TrieNodeCompressed(key, new FastBitSet(NSet), new FastBitSet(SSet));
        children = Arrays.copyOf(children, children.length + 1);
        children[children.length - 1] = node;
        return node;
    }

    void query(FastBitSet querySUnionN, FastBitSet queryN, int k,
            int budget, ArrayList<SetAndNd> out_list) {
        if (subtrieIntersectionOfNSets.cardinalityOfDifference(queryN) > k) {
            return;
        }
        if (querySUnionN.intersects(subtrieIntersectionOfSSets)) {
            return;
        }
        for (FastBitSet SSet : SSets) {
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

    private void printLatexBitset(FastBitSet bitset, String colour) {
        System.out.print("\\\\ [-1ex] \\scriptsize {\\color{" + colour + "} $");
        if (bitset.isEmpty()) {
            System.out.print("\\emptyset");
        } else {
            String comma = "";
            for (int v : bitset.toArray()) {
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
            for (FastBitSet SSet : SSets) {
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
