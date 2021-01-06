package bute;

import tw.exact.Graph;

class Dom {
    final ButeOptions options;
    final FastBitSet[] adjVvDominatedBy;
    final FastBitSet[] vvDominatedBy;
    final FastBitSet[] vvThatDominate;

    Dom(Graph g, ButeOptions options) {
        this.options = options;
        int n = g.n;
        adjVvDominatedBy = new FastBitSet[n];
        vvDominatedBy = new FastBitSet[n];
        vvThatDominate = new FastBitSet[n];
        for (int i=0; i<n; i++) {
            adjVvDominatedBy[i] = new FastBitSet(n);
            vvDominatedBy[i] = new FastBitSet(n);
            vvThatDominate[i] = new FastBitSet(n);
        }

        if (!options.useDomination) {
            return;
        }

        for (int v=0; v<n; v++) {
            for (int w=0; w<n; w++) {
                if (w != v) {
                    FastBitSet v_row = new FastBitSet(n, g.neighborSet[v]);
                    v_row.set(v);
                    v_row.set(w);
                    FastBitSet w_row = new FastBitSet(n, g.neighborSet[w]);
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
