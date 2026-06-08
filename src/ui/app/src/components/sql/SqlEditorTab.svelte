<script lang="ts">
	import SqlEditor from './SqlEditor.svelte';
	import QueryToolbar from './QueryToolbar.svelte';
	import QueryHistory from './QueryHistory.svelte';
	import ResultsTable from '../results/ResultsTable.svelte';
	import ResultsStatusBar from '../results/ResultsStatusBar.svelte';
	import ErrorDisplay from '../results/ErrorDisplay.svelte';
	import { executeSQL } from '../../lib/api';
	import type { QueryResult, HistoryEntry } from '../../lib/types';
	import { getTablesState, refreshTables } from '../../lib/stores/tables.svelte';

	let { initialQuery = '' }: { initialQuery?: string } = $props();

	let editorApi: { getValue: () => string; setValue: (v: string) => void } | null = null;
	let executing = $state(false);
	let results = $state<{ result: QueryResult; error: string | null; duration: number }[]>([]);
	let showHistory = $state(false);
	let history = $state<HistoryEntry[]>(loadHistory());

	const tablesState = getTablesState();
	let tableNames = $derived(tablesState.tables.map(t => t.name));

	function loadHistory(): HistoryEntry[] {
		try {
			const raw = localStorage.getItem('manticore-query-history');
			return raw ? JSON.parse(raw) : [];
		} catch { return []; }
	}

	function saveHistory(entries: HistoryEntry[]) {
		localStorage.setItem('manticore-query-history', JSON.stringify(entries.slice(0, 100)));
	}

	async function handleExecute(sql?: string) {
		const fullQuery = sql || editorApi?.getValue() || '';
		if (!fullQuery.trim()) return;

		// Split by semicolons for multi-statement support
		const statements = fullQuery.split(';').map(s => s.trim()).filter(s => s.length > 0);

		executing = true;
		results = [];
		const start = performance.now();

		const newResults: typeof results = [];
		let allSuccess = true;

		for (const stmt of statements) {
			try {
				const result = await executeSQL(stmt);
				newResults.push({ result, error: null, duration: 0 });
			} catch (e) {
				const msg = e instanceof Error ? e.message : String(e);
				newResults.push({ result: { columns: [], data: [], total: 0, error: msg, warning: '' }, error: msg, duration: 0 });
				allSuccess = false;
			}
		}

		const totalDuration = performance.now() - start;
		if (newResults.length > 0) {
			newResults[newResults.length - 1].duration = totalDuration;
		}

		results = newResults;
		executing = false;

		// Refresh table list if query might have changed schema
		const ddlPattern = /^\s*(create|drop|alter|truncate|attach|import)\b/i;
		if (statements.some(s => ddlPattern.test(s))) {
			refreshTables();
		}

		const entry: HistoryEntry = { query: fullQuery, timestamp: Date.now(), success: allSuccess, duration: totalDuration };
		history = [entry, ...history];
		saveHistory(history);
	}

	function handleClear() {
		editorApi?.setValue('');
		results = [];
	}

	function selectFromHistory(query: string) {
		editorApi?.setValue(query);
		showHistory = false;
	}

	function handleEditorReady(api: { getValue: () => string; setValue: (v: string) => void }) {
		editorApi = api;
	}
</script>

<div class="sql-tab">
	<div class="editor-section">
		<SqlEditor
			onExecute={handleExecute}
			{tableNames}
			initialValue={initialQuery}
			onReady={handleEditorReady}
		/>
		<QueryToolbar
			onExecute={() => handleExecute()}
			onClear={handleClear}
			{executing}
			onToggleHistory={() => showHistory = !showHistory}
		/>
	</div>

	<div class="results-section">
		{#if executing}
			<div class="results-loading">
				<div class="spinner"></div>
				Running query...
			</div>
		{:else if results.length === 0}
			<div class="results-placeholder">
				<div class="placeholder-icon">
					<svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="var(--color-border)" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">
						<polyline points="4 17 10 11 4 5"></polyline>
						<line x1="12" y1="19" x2="20" y2="19"></line>
					</svg>
				</div>
				<span class="placeholder-text">Run a query to see results</span>
				<span class="placeholder-hint">Ctrl+Enter to execute</span>
			</div>
		{:else}
			{#each results as r, i (i)}
				{#if r.error}
					<ErrorDisplay message={r.error} />
				{:else if r.result.columns.length > 0}
					<ResultsTable columns={r.result.columns} data={r.result.data} />
					<ResultsStatusBar
						rowCount={r.result.data.length}
						total={r.result.total}
						duration={r.duration}
						warning={r.result.warning}
					/>
				{:else}
					<div class="results-ok">
						<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="var(--color-success)" stroke-width="2.5">
							<polyline points="20 6 9 17 4 12"></polyline>
						</svg>
						Query OK
						{#if r.result.total > 0}
							<span class="affected-rows">({r.result.total} rows affected)</span>
						{/if}
					</div>
				{/if}
			{/each}
		{/if}
	</div>

	{#if showHistory}
		<QueryHistory
			entries={history}
			onSelect={selectFromHistory}
			onClose={() => showHistory = false}
		/>
	{/if}
</div>

<style>
	.sql-tab {
		display: flex;
		flex-direction: column;
		flex: 1;
		overflow: hidden;
		padding: var(--space-md);
		position: relative;
		gap: 0;
	}
	.editor-section {
		display: flex;
		flex-direction: column;
		min-height: 120px;
		height: 40%;
		flex-shrink: 0;
	}
	.results-section {
		display: flex;
		flex-direction: column;
		flex: 1;
		overflow: auto;
		min-height: 0;
		gap: var(--space-sm);
	}
	.results-loading {
		display: flex;
		align-items: center;
		gap: var(--space-sm);
		padding: var(--space-xl);
		color: var(--color-text-secondary);
		font-size: var(--font-size-sm);
	}
	.spinner {
		width: 16px;
		height: 16px;
		border: 2px solid var(--color-border);
		border-top-color: var(--color-primary);
		border-radius: 50%;
		animation: spin 0.6s linear infinite;
	}
	@keyframes spin {
		to { transform: rotate(360deg); }
	}
	.results-ok {
		display: flex;
		align-items: center;
		gap: var(--space-sm);
		padding: var(--space-md);
		background: #f0f7ec;
		border: 1px solid #c8dfc0;
		border-radius: var(--radius-md);
		color: var(--color-success);
		font-size: var(--font-size-sm);
		font-weight: 500;
	}
	.affected-rows {
		color: var(--color-text-secondary);
		font-weight: 400;
	}
	.results-placeholder {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		flex: 1;
		gap: var(--space-sm);
		color: var(--color-text-muted);
	}
	.placeholder-text {
		font-size: var(--font-size-md);
	}
	.placeholder-hint {
		font-size: var(--font-size-xs);
		opacity: 0.7;
	}
</style>
