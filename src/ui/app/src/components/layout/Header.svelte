<script lang="ts">
	let { connected = true }: { connected?: boolean } = $props();

	let dark = $state(localStorage.getItem('manticore-theme') === 'dark');

	function toggleTheme() {
		dark = !dark;
		document.documentElement.setAttribute('data-theme', dark ? 'dark' : 'light');
		localStorage.setItem('manticore-theme', dark ? 'dark' : 'light');
	}

	// Apply saved theme on load
	$effect(() => {
		if (dark) {
			document.documentElement.setAttribute('data-theme', 'dark');
		}
	});
</script>

<header class="header">
	<div class="header-left">
		<img class="logo" src="/ui/logo.svg" alt="Manticore Search" />
	</div>
	<div class="header-right">
		<span class="status" class:connected class:disconnected={!connected}>
			<span class="status-dot"></span>
			{connected ? 'Connected' : 'Disconnected'}
		</span>
		<button class="theme-toggle" onclick={toggleTheme} title={dark ? 'Switch to light mode' : 'Switch to dark mode'}>
			{#if dark}
				<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
					<circle cx="12" cy="12" r="5"></circle>
					<line x1="12" y1="1" x2="12" y2="3"></line>
					<line x1="12" y1="21" x2="12" y2="23"></line>
					<line x1="4.22" y1="4.22" x2="5.64" y2="5.64"></line>
					<line x1="18.36" y1="18.36" x2="19.78" y2="19.78"></line>
					<line x1="1" y1="12" x2="3" y2="12"></line>
					<line x1="21" y1="12" x2="23" y2="12"></line>
					<line x1="4.22" y1="19.78" x2="5.64" y2="18.36"></line>
					<line x1="18.36" y1="5.64" x2="19.78" y2="4.22"></line>
				</svg>
			{:else}
				<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
					<path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"></path>
				</svg>
			{/if}
		</button>
	</div>
</header>

<style>
	.header {
		height: var(--header-height);
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 0 var(--space-lg);
		background: var(--color-bg);
		border-bottom: 1px solid var(--color-border);
		flex-shrink: 0;
	}
	.header-left {
		display: flex;
		align-items: center;
	}
	.logo {
		height: 22px;
		width: auto;
	}
	.header-right {
		display: flex;
		align-items: center;
		gap: var(--space-md);
	}
	.status {
		display: flex;
		align-items: center;
		gap: 6px;
		font-size: var(--font-size-sm);
		color: var(--color-text-secondary);
	}
	.status-dot {
		width: 7px;
		height: 7px;
		border-radius: 50%;
		background: var(--color-text-muted);
	}
	.connected .status-dot {
		background: var(--color-success);
	}
	.disconnected .status-dot {
		background: var(--color-error);
	}
	.theme-toggle {
		padding: 6px;
		border-radius: var(--radius-sm);
		color: var(--color-text-secondary);
		display: flex;
		align-items: center;
	}
	.theme-toggle:hover {
		background: var(--color-bg-hover);
		color: var(--color-text);
	}
</style>
