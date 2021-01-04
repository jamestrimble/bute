package bute;

class TreedepthResult {
    private int depth;
    private int[] parent;

    TreedepthResult(int depth, int[] parent) {
        this.depth = depth;
        this.parent = parent;
    }

    int getDepth() {
        return depth;
    }

    int[] getParent() {
        return parent;
    }
}
