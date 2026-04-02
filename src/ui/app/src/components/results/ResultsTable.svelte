<script lang="ts">
	import type { ColumnDef } from '../../lib/types';

	let { columns, data }: {
		columns: ColumnDef[];
		data: Record<string, unknown>[];
	} = $props();

	let sortCol = $state<string | null>(null);
	let sortDir = $state<'asc' | 'desc'>('asc');
	let expandedCell = $state<string | null>(null);

	function toggleSort(colName: string) {
		if (sortCol === colName) {
			sortDir = sortDir === 'asc' ? 'desc' : 'asc';
		} else {
			sortCol = colName;
			sortDir = 'asc';
		}
	}

	let sortedData = $derived.by(() => {
		if (!sortCol) return data;
		const col = sortCol;
		const dir = sortDir === 'asc' ? 1 : -1;
		return [...data].sort((a, b) => {
			const va = a[col], vb = b[col];
			if (va === null || va === undefined) return 1;
			if (vb === null || vb === undefined) return -1;
			if (typeof va === 'number' && typeof vb === 'number') return (va - vb) * dir;
			return String(va).localeCompare(String(vb)) * dir;
		});
	});

	function isJson(value: unknown): boolean {
		return value !== null && typeof value === 'object';
	}

	function formatCell(value: unknown): string {
		if (value === null || value === undefined) return 'NULL';
		if (typeof value === 'object') return JSON.stringify(value);
		return String(value);
	}

	function formatJsonPretty(value: unknown): string {
		return JSON.stringify(value, null, 2);
	}

	function cellClass(value: unknown): string {
		if (value === null || value === undefined) return 'cell-null';
		if (typeof value === 'number') return 'cell-number';
		if (typeof value === 'object') return 'cell-json';
		return '';
	}

	function cellKey(rowIdx: number, colName: string): string {
		return `${rowIdx}:${colName}`;
	}

	function toggleExpand(key: string) {
		expandedCell = expandedCell === key ? null : key;
	}

	function exportData(format: 'csv' | 'json') {
		let content: string;
		let mime: string;
		let ext: string;

		if (format === 'csv') {
			const header = columns.map(c => `"${c.name}"`).join(',');
			const rows = data.map(row =>
				columns.map(c => {
					const v = row[c.name];
					if (v === null || v === undefined) return '';
					if (typeof v === 'object') return `"${JSON.stringify(v).replace(/"/g, '""')}"`;
					return `"${String(v).replace(/"/g, '""')}"`;
				}).join(',')
			);
			content = [header, ...rows].join('\n');
			mime = 'text/csv';
			ext = 'csv';
		} else {
			content = JSON.stringify(data, null, 2);
			mime = 'application/json';
			ext = 'json';
		}

		const blob = new Blob([content], { type: mime });
		const url = URL.createObjectURL(blob);
		const a = document.createElement('a');
		a.href = url;
		a.download = `results.${ext}`;
		a.click();
		URL.revokeObjectURL(url);
	}
</script>

<div class="table-toolbar">
	<span class="table-info">{data.length} row{data.length !== 1 ? 's' : ''}</span>
	<div class="export-btns">
		<button class="export-btn" onclick={() => exportData('csv')} title="Export CSV">
			<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
				<path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path>
				<polyline points="7 10 12 15 17 10"></polyline>
				<line x1="12" y1="15" x2="12" y2="3"></line>
			</svg>
			CSV
		</button>
		<button class="export-btn" onclick={() => exportData('json')} title="Export JSON">
			<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
				<path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path>
				<polyline points="7 10 12 15 17 10"></polyline>
				<line x1="12" y1="15" x2="12" y2="3"></line>
			</svg>
			JSON
		</button>
	</div>
</div>
<div class="table-wrap">
	<table>
		<thead>
			<tr>
				{#each columns as col (col.name)}
					<th
						title="{col.name} ({col.type})"
						onclick={() => toggleSort(col.name)}
						class:sorted={sortCol === col.name}
					>
						<span class="th-content">
							{col.name}
							{#if sortCol === col.name}
								<span class="sort-arrow">{sortDir === 'asc' ? '\u2191' : '\u2193'}</span>
							{/if}
						</span>
						<span class="th-type">{col.type}</span>
					</th>
				{/each}
			</tr>
		</thead>
		<tbody>
			{#each sortedData as row, i (i)}
				<tr>
					{#each columns as col (col.name)}
						{@const key = cellKey(i, col.name)}
						{@const val = row[col.name]}
						<td
							class={cellClass(val)}
							class:expanded={expandedCell === key}
							onclick={() => isJson(val) ? toggleExpand(key) : null}
						>
							{#if expandedCell === key && isJson(val)}
								<pre class="json-expanded">{formatJsonPretty(val)}</pre>
							{:else}
								{formatCell(val)}
							{/if}
						</td>
					{/each}
				</tr>
			{/each}
		</tbody>
	</table>
</div>

<style>
	.table-toolbar {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: var(--space-xs) 0;
		flex-shrink: 0;
	}
	.table-info {
		font-size: var(--font-size-xs);
		color: var(--color-text-muted);
	}
	.export-btns {
		display: flex;
		gap: 4px;
	}
	.export-btn {
		display: inline-flex;
		align-items: center;
		gap: 4px;
		padding: 2px 8px;
		font-size: var(--font-size-xs);
		color: var(--color-text-secondary);
		border: 1px solid var(--color-border-light);
		border-radius: var(--radius-sm);
		background: var(--color-bg);
	}
	.export-btn:hover {
		background: var(--color-bg-hover);
		border-color: var(--color-border);
	}
	.table-wrap {
		overflow: auto;
		flex: 1;
		border: 1px solid var(--color-border);
		border-radius: var(--radius-md);
		background: var(--color-bg);
	}
	table {
		width: 100%;
		border-collapse: collapse;
		font-size: var(--font-size-sm);
		font-family: var(--font-mono);
	}
	thead {
		position: sticky;
		top: 0;
		z-index: 1;
	}
	th {
		background: var(--color-bg-tertiary);
		padding: 6px 12px;
		text-align: left;
		font-family: var(--font-sans);
		font-size: var(--font-size-xs);
		color: var(--color-text-secondary);
		border-bottom: 1px solid var(--color-border);
		white-space: nowrap;
		cursor: pointer;
		user-select: none;
		transition: background 0.1s;
	}
	th:hover {
		background: var(--color-bg-hover);
		color: var(--color-text);
	}
	th.sorted {
		color: var(--color-primary);
	}
	.th-content {
		display: flex;
		align-items: center;
		gap: 4px;
		font-weight: 600;
		text-transform: uppercase;
		letter-spacing: 0.3px;
	}
	.th-type {
		display: block;
		font-size: 10px;
		font-weight: 400;
		color: var(--color-text-muted);
		text-transform: none;
		letter-spacing: 0;
		margin-top: 1px;
	}
	.sort-arrow {
		font-size: 11px;
	}
	td {
		padding: 5px 12px;
		border-bottom: 1px solid var(--color-border-light);
		max-width: 400px;
		overflow: hidden;
		text-overflow: ellipsis;
		white-space: nowrap;
	}
	td.expanded {
		max-width: none;
		white-space: pre;
		overflow: visible;
	}
	tr:hover td {
		background: var(--color-primary-subtle);
	}
	.cell-null {
		color: var(--color-text-muted);
		font-style: italic;
	}
	.cell-number {
		text-align: right;
		font-variant-numeric: tabular-nums;
	}
	.cell-json {
		cursor: pointer;
		color: var(--color-info);
	}
	.cell-json:hover {
		text-decoration: underline;
	}
	.json-expanded {
		font-size: var(--font-size-xs);
		line-height: 1.4;
		margin: 0;
		color: var(--color-text);
		white-space: pre-wrap;
		word-break: break-word;
	}
</style>
