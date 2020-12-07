package bute;

import java.util.*;

import tw.exact.Graph;

class ButeSolver {
    Graph g;
    int n;

    ButeSolver(Graph g) {
        this.g = g;
        this.n = g.n;
    }

    TreedepthResult solve() {
        Dom dom = new Dom(g);
        for (int target=0; target<=n; target++) {
            TreedepthResult result =
                    new ButeDecisionProblemSolver(g, dom, target).solve();
            if (result != null) {
                return result;
            }
        }
        return null;   // never reached
    }
}
