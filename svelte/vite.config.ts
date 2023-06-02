import { sveltekit } from '@sveltejs/kit/vite';
import { defineConfig } from 'vite';

export default defineConfig({
	plugins: [sveltekit()],
	
	// https://medium.com/mkdir-awesome/two-easy-ways-to-publish-your-svelte-project-on-github-pages-c8eaca2b6225#c71c
	build: {
		outDir: "../docs/",
		emptyOutDir: false
	}
});
