package bute;

import tw.exact.Graph;

class ButeSolver {
    final Graph g;
    final int n;
    final ButeOptions options;

    ButeSolver(Graph g, ButeOptions options) {
        this.g = g;
        this.n = g.n;
        this.options = options;
    }

    TreedepthResult solve() {
        Dom dom = new Dom(g, options);
        for (int target=0; target<=n; target++) {
            TreedepthResult result =
                    new ButeDecisionProblemSolver(g, dom, target, options).solve();
            if (result != null) {
                return result;
            }
        }
        return null;   // never reached
    }
}
