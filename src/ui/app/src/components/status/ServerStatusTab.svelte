<script lang="ts">
	import { onMount } from 'svelte';
	import ResultsTable from '../results/ResultsTable.svelte';
	import ErrorDisplay from '../results/ErrorDisplay.svelte';
	import { serverStatus, serverVariables } from '../../lib/api';
	import type { QueryResult } from '../../lib/types';

	let statusResult = $state<QueryResult | null>(null);
	let variablesResult = $state<QueryResult | null>(null);
	let error = $state<string | null>(null);
	let activeSection = $state<'status' | 'variables'>('status');
	let autoRefresh = $state(false);
	let refreshInterval: ReturnType<typeof setInterval> | null = null;
	let filter = $state('');

	async function loadData() {
		try {
			error = null;
			if (activeSection === 'status') {
				statusResult = await serverStatus();
			} else {
				variablesResult = await serverVariables();
			}
		} catch (e) {
			error = e instanceof Error ? e.message : String(e);
		}
	}

	function toggleAutoRefresh() {
		autoRefresh = !autoRefresh;
		if (autoRefresh) {
			refreshInterval = setInterval(loadData, 5000);
		} else if (refreshInterval) {
			clearInterval(refreshInterval);
			refreshInterval = null;
		}
	}

	function getFilteredData(result: QueryResult | null): Record<string, unknown>[] {
		if (!result) return [];
		if (!filter) return result.data;
		const lf = filter.toLowerCase();
		return result.data.filter(row =>
			Object.values(row).some(v => String(v).toLowerCase().includes(lf))
		);
	}

	onMount(() => {
		loadData();
		return () => {
			if (refreshInterval) clearInterval(refreshInterval);
		};
	});
</script>

<div class="status-tab">
	<div class="status-header">
		<div class="section-tabs">
			<button class="section-tab" class:active={activeSection === 'status'} onclick={() => { activeSection = 'status'; loadData(); }}>
				Server Status
			</button>
			<button class="section-tab" class:active={activeSection === 'variables'} onclick={() => { activeSection = 'variables'; loadData(); }}>
				Variables
			</button>
		</div>
		<div class="status-actions">
			<input
				class="filter-input"
				type="text"
				placeholder="Filter..."
				bind:value={filter}
			/>
			<button class="btn btn-ghost" onclick={loadData}>Refresh</button>
			<button class="btn" class:btn-active={autoRefresh} onclick={toggleAutoRefresh}>
				Auto {autoRefresh ? 'ON' : 'OFF'}
			</button>
		</div>
	</div>

	{#if error}
		<ErrorDisplay message={error} />
	{/if}

	<div class="status-content">
		{#if activeSection === 'status' && statusResult}
			<ResultsTable columns={statusResult.columns} data={getFilteredData(statusResult)} />
		{:else if activeSection === 'variables' && variablesResult}
			<ResultsTable columns={variablesResult.columns} data={getFilteredData(variablesResult)} />
		{/if}
	</div>
</div>

<style>
	.status-tab {
		display: flex;
		flex-direction: column;
		flex: 1;
		overflow: hidden;
		padding: var(--space-md);
	}
	.status-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding-bottom: var(--space-md);
		flex-shrink: 0;
	}
	.section-tabs {
		display: flex;
	}
	.section-tab {
		padding: var(--space-sm) var(--space-lg);
		font-size: var(--font-size-sm);
		color: var(--color-text-secondary);
		border-bottom: 2px solid transparent;
	}
	.section-tab:hover { color: var(--color-text); }
	.section-tab.active {
		color: var(--color-primary);
		border-bottom-color: var(--color-primary);
	}
	.status-actions {
		display: flex;
		gap: var(--space-sm);
		align-items: center;
	}
	.filter-input {
		padding: 4px 10px;
		border: 1px solid var(--color-border);
		border-radius: var(--radius-md);
		font-size: var(--font-size-sm);
		width: 160px;
		outline: none;
	}
	.filter-input:focus {
		border-color: var(--color-primary);
	}
	.btn {
		padding: 4px 10px;
		border-radius: var(--radius-md);
		font-size: var(--font-size-sm);
		color: var(--color-text-secondary);
	}
	.btn-ghost:hover { background: var(--color-bg-hover); }
	.btn-active {
		background: var(--color-primary-light);
		color: var(--color-primary);
	}
	.status-content {
		flex: 1;
		overflow: hidden;
		display: flex;
		flex-direction: column;
	}
</style>
