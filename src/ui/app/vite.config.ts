import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';

export default defineConfig({
	plugins: [svelte()],
	base: '/ui/',
	build: {
		outDir: 'dist',
		assetsDir: 'assets',
		assetsInlineLimit: 4096,
	},
	server: {
		proxy: {
			'/cli_json': 'http://localhost:9308',
			'/cli': 'http://localhost:9308',
			'/sql': 'http://localhost:9308',
			'/search': 'http://localhost:9308',
			'/json': 'http://localhost:9308',
			'/insert': 'http://localhost:9308',
			'/delete': 'http://localhost:9308',
			'/update': 'http://localhost:9308',
			'/bulk': 'http://localhost:9308',
		},
	},
});
