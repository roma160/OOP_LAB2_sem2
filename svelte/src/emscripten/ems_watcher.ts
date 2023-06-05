interface WatcherCallback {
    onElementOut(element: Element): void;
    onElementIn(element: Element): void;
}

class Watcher {
    #observer: IntersectionObserver;
    #callbacks: Map<Element, WatcherCallback>;

    constructor() {
        this.#callbacks = new Map<Element, WatcherCallback>();

        const self = this.#callbacks;
        this.#observer = new IntersectionObserver(
        function (entries: IntersectionObserverEntry[]){
            for (const entry of entries) {
                const element = entry.target;
                if (!self.has(element))
                    continue;
                const callback = self.get(element);
                if (entry.intersectionRatio == 0) {
                    callback?.onElementOut(element);
                }
                else {
                    callback?.onElementIn(element);
                }
            }
        });
    }

    addElement(element: Element, callback: WatcherCallback) {
        this.#callbacks.set(element, callback);
        this.#observer.observe(element);
    }

    removeElement(element: Element) {
        this.#observer.unobserve(element);
        this.#callbacks.delete(element);
    }
}

const watcher = new Watcher();
export default watcher;
export type { WatcherCallback };