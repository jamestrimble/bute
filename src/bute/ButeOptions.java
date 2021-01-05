package bute;

class ButeOptions {
    boolean useTrie;
    boolean useDomination;
    boolean lookForTopChain;
    boolean vertexDriven;

    public ButeOptions(boolean useTrie, boolean useDomination,
            boolean lookForTopChain, boolean vertexDriven) {
        this.useTrie = useTrie;
        this.useDomination = useDomination;
        this.lookForTopChain = lookForTopChain;
        this.vertexDriven = vertexDriven;
    }
}
