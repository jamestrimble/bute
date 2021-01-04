package bute;

class TreedepthResult {
    private final boolean satisfied;
    private int depth;
    private int[] parent;

    TreedepthResult() {
        satisfied = false;
    }

    TreedepthResult(int depth, int[] parent) {
        satisfied = true;
        this.depth = depth;
        this.parent = parent;
    }

    boolean isSatisfied() {
        return satisfied;
    }

    int getDepth() {
        return depth;
    }

    int[] getParent() {
        return parent;
    }
}
