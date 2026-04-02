<script lang="ts">
	import ResultsTable from '../results/ResultsTable.svelte';
	import ErrorDisplay from '../results/ErrorDisplay.svelte';
	import { describeTable, tableStatus, executeSQL } from '../../lib/api';
	import { refreshTables } from '../../lib/stores/tables.svelte';
	import type { QueryResult } from '../../lib/types';

	let { tableName, onOpenQuery }: {
		tableName: string;
		onOpenQuery: (sql: string) => void;
	} = $props();

	let schema = $state<QueryResult | null>(null);
	let status = $state<QueryResult | null>(null);
	let preview = $state<QueryResult | null>(null);
	let error = $state<string | null>(null);
	let activeSection = $state<'schema' | 'status' | 'data'>('schema');
	let dropping = $state(false);

	$effect(() => {
		loadTableInfo(tableName);
	});

	async function loadTableInfo(name: string) {
		error = null;
		try {
			const [s, st, p] = await Promise.all([
				describeTable(name),
				tableStatus(name),
				executeSQL(`SELECT * FROM ${name} LIMIT 20`),
			]);
			schema = s;
			status = st;
			preview = p;
		} catch (e) {
			error = e instanceof Error ? e.message : String(e);
		}
	}

	async function handleDrop() {
		if (!confirm(`Drop table "${tableName}"? This cannot be undone.`)) return;
		dropping = true;
		try {
			await executeSQL(`DROP TABLE ${tableName}`);
			await refreshTables();
		} catch (e) {
			error = e instanceof Error ? e.message : String(e);
		} finally {
			dropping = false;
		}
	}

	async function handleOptimize() {
		try {
			await executeSQL(`OPTIMIZE TABLE ${tableName}`);
			await loadTableInfo(tableName);
		} catch (e) {
			error = e instanceof Error ? e.message : String(e);
		}
	}
</script>

<div class="table-detail">
	<div class="detail-header">
		<h2 class="detail-title">{tableName}</h2>
		<div class="detail-actions">
			<button class="btn btn-ghost" onclick={() => onOpenQuery(`SELECT * FROM ${tableName} LIMIT 100`)}>
				Query
			</button>
			<button class="btn btn-ghost" onclick={handleOptimize}>Optimize</button>
			<button class="btn btn-danger" onclick={handleDrop} disabled={dropping}>
				{dropping ? 'Dropping...' : 'Drop'}
			</button>
		</div>
	</div>

	{#if error}
		<ErrorDisplay message={error} />
	{/if}

	<div class="section-tabs">
		<button class="section-tab" class:active={activeSection === 'schema'} onclick={() => activeSection = 'schema'}>Schema</button>
		<button class="section-tab" class:active={activeSection === 'data'} onclick={() => activeSection = 'data'}>Data Preview</button>
		<button class="section-tab" class:active={activeSection === 'status'} onclick={() => activeSection = 'status'}>Status</button>
	</div>

	<div class="section-content">
		{#if activeSection === 'schema' && schema}
			<ResultsTable columns={schema.columns} data={schema.data} />
		{:else if activeSection === 'data' && preview}
			<ResultsTable columns={preview.columns} data={preview.data} />
		{:else if activeSection === 'status' && status}
			<ResultsTable columns={status.columns} data={status.data} />
		{/if}
	</div>
</div>

<style>
	.table-detail {
		display: flex;
		flex-direction: column;
		flex: 1;
		overflow: hidden;
		padding: var(--space-md);
	}
	.detail-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding-bottom: var(--space-md);
		flex-shrink: 0;
	}
	.detail-title {
		font-size: var(--font-size-xl);
		font-weight: 600;
	}
	.detail-actions {
		display: flex;
		gap: var(--space-sm);
	}
	.btn {
		padding: 5px 12px;
		border-radius: var(--radius-md);
		font-size: var(--font-size-sm);
		font-weight: 500;
	}
	.btn-ghost {
		color: var(--color-text-secondary);
	}
	.btn-ghost:hover {
		background: var(--color-bg-hover);
		color: var(--color-text);
	}
	.btn-danger {
		color: var(--color-error);
	}
	.btn-danger:hover:not(:disabled) {
		background: #fdf0ef;
	}
	.section-tabs {
		display: flex;
		gap: 0;
		border-bottom: 1px solid var(--color-border);
		flex-shrink: 0;
	}
	.section-tab {
		padding: var(--space-sm) var(--space-lg);
		font-size: var(--font-size-sm);
		color: var(--color-text-secondary);
		border-bottom: 2px solid transparent;
	}
	.section-tab:hover {
		color: var(--color-text);
	}
	.section-tab.active {
		color: var(--color-primary);
		border-bottom-color: var(--color-primary);
	}
	.section-content {
		flex: 1;
		overflow: hidden;
		display: flex;
		flex-direction: column;
		padding-top: var(--space-md);
	}
</style>
