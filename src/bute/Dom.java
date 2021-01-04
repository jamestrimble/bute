package bute;

import tw.exact.Graph;
import tw.exact.XBitSet;

class Dom {
    final XBitSet[] adjVvDominatedBy;
    final XBitSet[] vvDominatedBy;
    final XBitSet[] vvThatDominate;
    Dom(Graph g) {
        int n = g.n;
        adjVvDominatedBy = new XBitSet[n];
        vvDominatedBy = new XBitSet[n];
        vvThatDominate = new XBitSet[n];
        for (int i=0; i<n; i++) {
            adjVvDominatedBy[i] = new XBitSet();
            vvDominatedBy[i] = new XBitSet();
            vvThatDominate[i] = new XBitSet();
        }

        for (int v=0; v<n; v++) {
            for (int w=0; w<n; w++) {
                if (w != v) {
                    XBitSet v_row = (XBitSet) g.neighborSet[v].clone();
                    v_row.set(v);
                    v_row.set(w);
                    XBitSet w_row = (XBitSet) g.neighborSet[w].clone();
                    w_row.set(v);
                    w_row.set(w);
                    if (!w_row.isSuperset(v_row)) {
                        continue;
                    }
                    if (v_row.equals(w_row) && v >= w) {
                        continue;
                    }
                    vvDominatedBy[w].set(v);
                    vvThatDominate[v].set(w);
                    if (g.neighborSet[w].get(v)) {
                        adjVvDominatedBy[w].set(v);
                    }
                }
            }
        }
    }
}
