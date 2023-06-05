<p bind:this={root}>
Hello world!:
</p>

<script lang="ts">
    import { onMount } from 'svelte';

    let root: Element;

    onMount(async () => {
        let watcher = (await import("./ems_watcher")).default;
        watcher.addElement(root, {
            onElementIn(element) {
                console.log("I am in!");
            },
            onElementOut(element) {
                console.log("I am out!");
            },
        });
    })
</script>