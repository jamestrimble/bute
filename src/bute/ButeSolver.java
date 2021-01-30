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
        if (g.n == 0) {
            return new TreedepthResult(0, new int[0]);
        }
        Dom dom = new Dom(g, options);
        for (int target=1; target<=n; target++) {
            long prevHelperCalls = Stats.helperCalls;
            TreedepthResult result =
                    new ButeDecisionProblemSolver(g, dom, target, options).solve();
            if (result != null) {
                Stats.lastDecisionProblemHelperCalls = Stats.helperCalls - prevHelperCalls;
                return result;
            }
        }
        return null;   // never reached
    }
}
