package bute;

class ButeOptions {
    boolean useTrie;
    boolean useDomination;
    boolean vertexDriven;

    public ButeOptions(boolean useTrie, boolean useDomination, boolean vertexDriven) {
        this.useTrie = useTrie;
        this.useDomination = useDomination;
        this.vertexDriven = vertexDriven;
    }
}
