<script lang="ts">
	import { onMount } from 'svelte';
	import { getTablesState, refreshTables } from '../../lib/stores/tables.svelte';

	let { onSelectTable, selectedTable = '', onCreateTable }: {
		onSelectTable: (name: string) => void;
		selectedTable?: string;
		onCreateTable: () => void;
	} = $props();

	const state = getTablesState();

	onMount(() => {
		const interval = setInterval(refreshTables, 15000);
		return () => clearInterval(interval);
	});

	function typeIcon(type: string): string {
		switch (type) {
			case 'rt': return 'RT';
			case 'distributed': return 'DI';
			case 'percolate': return 'PQ';
			default: return 'RT';
		}
	}

	function typeColor(type: string): string {
		switch (type) {
			case 'rt': return 'var(--color-primary)';
			case 'distributed': return 'var(--color-info)';
			case 'percolate': return 'var(--color-warning)';
			default: return 'var(--color-text-secondary)';
		}
	}

	function formatCount(n?: number): string {
		if (n === undefined) return '';
		if (n >= 1_000_000) return `${(n / 1_000_000).toFixed(1)}M`;
		if (n >= 1_000) return `${(n / 1_000).toFixed(1)}K`;
		return String(n);
	}

	$effect(() => {
		refreshTables();
	});
</script>

<aside class="sidebar">
	<div class="sidebar-header">
		<span class="sidebar-title">Tables</span>
		<div class="sidebar-actions">
			<button class="sidebar-btn" onclick={onCreateTable} title="Create table">
				<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<line x1="12" y1="5" x2="12" y2="19"></line>
					<line x1="5" y1="12" x2="19" y2="12"></line>
				</svg>
			</button>
			<button class="sidebar-btn" onclick={() => refreshTables()} title="Refresh tables">
				<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
					<polyline points="23 4 23 10 17 10"></polyline>
					<polyline points="1 20 1 14 7 14"></polyline>
					<path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"></path>
				</svg>
			</button>
		</div>
	</div>

	{#if state.loading && state.tables.length === 0}
		<div class="sidebar-message">Loading...</div>
	{:else if state.error}
		<div class="sidebar-message error">{state.error}</div>
	{:else if state.tables.length === 0}
		<div class="sidebar-message">No tables found</div>
	{:else}
		<ul class="table-list">
			{#each state.tables as table (table.name)}
				<li>
					<button
						class="table-item"
						class:active={selectedTable === table.name}
						onclick={() => onSelectTable(table.name)}
					>
						<span class="table-type-badge" style:background={typeColor(table.type)}>{typeIcon(table.type)}</span>
						<span class="table-name">{table.name}</span>
						{#if table.count !== undefined}
							<span class="table-count">{formatCount(table.count)}</span>
						{/if}
					</button>
				</li>
			{/each}
		</ul>
	{/if}
</aside>

<style>
	.sidebar {
		width: var(--sidebar-width);
		min-width: var(--sidebar-width);
		background: var(--color-bg-secondary);
		border-right: 1px solid var(--color-border);
		display: flex;
		flex-direction: column;
		overflow: hidden;
	}
	.sidebar-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: var(--space-md) var(--space-lg);
		border-bottom: 1px solid var(--color-border-light);
	}
	.sidebar-title {
		font-size: var(--font-size-sm);
		font-weight: 600;
		text-transform: uppercase;
		letter-spacing: 0.5px;
		color: var(--color-text-secondary);
	}
	.sidebar-actions {
		display: flex;
		gap: 2px;
	}
	.sidebar-btn {
		padding: var(--space-xs);
		border-radius: var(--radius-sm);
		color: var(--color-text-secondary);
		display: flex;
		align-items: center;
	}
	.sidebar-btn:hover {
		background: var(--color-bg-hover);
		color: var(--color-text);
	}
	.sidebar-message {
		padding: var(--space-lg);
		font-size: var(--font-size-sm);
		color: var(--color-text-secondary);
		text-align: center;
	}
	.sidebar-message.error {
		color: var(--color-error);
	}
	.table-list {
		list-style: none;
		overflow-y: auto;
		flex: 1;
		padding: var(--space-xs) 0;
	}
	.table-item {
		display: flex;
		align-items: center;
		gap: var(--space-sm);
		width: 100%;
		padding: 7px var(--space-lg);
		text-align: left;
		font-size: var(--font-size-md);
		border-radius: 0;
		transition: background 0.1s;
	}
	.table-item:hover {
		background: var(--color-bg-hover);
	}
	.table-item.active {
		background: var(--color-primary-light);
		color: var(--color-primary);
	}
	.table-type-badge {
		font-size: 9px;
		font-weight: 700;
		font-family: var(--font-mono);
		color: white;
		padding: 1px 4px;
		border-radius: 3px;
		flex-shrink: 0;
		line-height: 1.3;
	}
	.table-name {
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
		flex: 1;
	}
	.table-count {
		font-size: var(--font-size-xs);
		color: var(--color-text-muted);
		font-family: var(--font-mono);
		font-variant-numeric: tabular-nums;
		flex-shrink: 0;
	}
</style>
