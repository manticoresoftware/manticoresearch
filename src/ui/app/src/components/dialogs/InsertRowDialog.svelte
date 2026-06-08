<script lang="ts">
	import { executeSQL, describeTable } from '../../lib/api';
	import type { ColumnDef } from '../../lib/types';

	let { open = false, tableName = '', onClose }: {
		open: boolean;
		tableName: string;
		onClose: () => void;
	} = $props();

	let columns = $state<ColumnDef[]>([]);
	let values = $state<Record<string, string>>({});
	let valuesVersion = $state(0);
	let loading = $state(false);
	let executing = $state(false);
	let error = $state<string | null>(null);
	let success = $state(false);

	$effect(() => {
		if (open && tableName) {
			loadSchema();
		}
	});

	async function loadSchema() {
		loading = true;
		error = null;
		success = false;
		try {
			const result = await describeTable(tableName);
			columns = result.data
				.map(row => ({
					name: (row['Field'] || row['field'] || '') as string,
					type: (row['Type'] || row['type'] || '') as string,
				}));
			values = {};
			for (const col of columns) {
				values[col.name] = '';
			}
		} catch (e) {
			error = e instanceof Error ? e.message : String(e);
		} finally {
			loading = false;
		}
	}

	function updateValue(name: string, val: string) {
		values[name] = val;
		valuesVersion++;
	}

	function buildSQL(): string {
		void valuesVersion; // trigger reactivity
		const filled = columns.filter(c => String(values[c.name] ?? '').trim());
		if (!filled.length || !tableName) return '';

		const cols = filled.map(c => c.name).join(', ');
		const vals = filled.map(c => {
			const v = String(values[c.name] ?? '').trim();
			const t = c.type.toLowerCase();
			if (['integer', 'int', 'bigint', 'float', 'bool', 'timestamp'].some(n => t.includes(n))) {
				return v;
			}
			return `'${v.replace(/'/g, "\\'")}'`;
		}).join(', ');

		return `INSERT INTO ${tableName} (${cols}) VALUES (${vals})`;
	}

	let generatedSQL = $derived(buildSQL());

	async function handleInsert() {
		if (!generatedSQL) return;
		executing = true;
		error = null;
		success = false;
		try {
			await executeSQL(generatedSQL);
			success = true;
			// Reset values for next insert
			for (const col of columns) {
				values[col.name] = '';
			}
			valuesVersion++;
		} catch (e) {
			error = e instanceof Error ? e.message : String(e);
		} finally {
			executing = false;
		}
	}

	function handleClose() {
		error = null;
		success = false;
		onClose();
	}

	function copySQL() {
		if (generatedSQL) navigator.clipboard.writeText(generatedSQL);
	}

	function inputType(colType: string): string {
		const t = colType.toLowerCase();
		if (['integer', 'int', 'bigint', 'float', 'timestamp'].some(n => t.includes(n))) return 'number';
		return 'text';
	}

	function isTextArea(colType: string): boolean {
		const t = colType.toLowerCase();
		return t === 'text' || t === 'json' || t === 'string';
	}
</script>

{#if open}
<div class="overlay" onclick={handleClose} onkeydown={(e) => e.key === 'Escape' && handleClose()} role="dialog" tabindex="-1">
	<div class="dialog" onclick={(e) => e.stopPropagation()} role="document">
		<div class="dialog-header">
			<h2>Insert into <span class="table-badge">{tableName}</span></h2>
			<button class="close-btn" onclick={handleClose} title="Close">
				<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<line x1="18" y1="6" x2="6" y2="18"></line>
					<line x1="6" y1="6" x2="18" y2="18"></line>
				</svg>
			</button>
		</div>

		<div class="dialog-body">
			{#if loading}
				<div class="loading">Loading schema...</div>
			{:else if columns.length === 0}
				<div class="loading">No columns found</div>
			{:else}
				<div class="fields-list">
					{#each columns as col (col.name)}
						<div class="field-group">
							<label for="insert-{col.name}">
								<span class="field-label">{col.name}</span>
								<span class="field-type-hint">{col.type}{col.name === 'id' ? ' — optional, auto-generated if empty' : ''}</span>
							</label>
							{#if isTextArea(col.type)}
								<textarea
									id="insert-{col.name}"
									value={values[col.name] ?? ''}
									oninput={(e) => updateValue(col.name, e.currentTarget.value)}
									placeholder={col.name === 'id' ? 'auto-generated if empty' : col.type}
									rows="2"
									class:mono={col.type.toLowerCase() === 'json'}
								></textarea>
							{:else if col.type.toLowerCase() === 'bool'}
								<select id="insert-{col.name}" value={values[col.name] ?? ''} onchange={(e) => updateValue(col.name, e.currentTarget.value)}>
									<option value="">— select —</option>
									<option value="1">true (1)</option>
									<option value="0">false (0)</option>
								</select>
							{:else}
								<input
									id="insert-{col.name}"
									type="text"
									inputmode={inputType(col.type) === 'number' ? 'numeric' : 'text'}
									value={values[col.name] ?? ''}
									oninput={(e) => updateValue(col.name, e.currentTarget.value)}
									placeholder={col.name === 'id' ? 'auto-generated if empty' : col.type}
								/>
							{/if}
						</div>
					{/each}
				</div>

				{#if generatedSQL}
					<div class="sql-preview">
						<div class="sql-preview-header">
							<span class="sql-preview-label">Generated SQL</span>
							<button class="copy-btn" onclick={copySQL}>Copy</button>
						</div>
						<pre class="sql-preview-code">{generatedSQL}</pre>
					</div>
				{/if}
			{/if}

			{#if success}
				<div class="success-msg">Row inserted successfully</div>
			{/if}

			{#if error}
				<div class="dialog-error">{error}</div>
			{/if}
		</div>

		<div class="dialog-footer">
			<button class="btn btn-ghost" onclick={handleClose}>Close</button>
			<button class="btn btn-primary" onclick={handleInsert} disabled={!generatedSQL || executing}>
				{executing ? 'Inserting...' : 'Insert Row'}
			</button>
		</div>
	</div>
</div>
{/if}

<style>
	.overlay {
		position: fixed;
		inset: 0;
		background: rgba(0, 0, 0, 0.4);
		display: flex;
		align-items: center;
		justify-content: center;
		z-index: 100;
	}
	.dialog {
		background: var(--color-bg);
		border-radius: var(--radius-lg);
		box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
		width: 580px;
		max-width: 90vw;
		max-height: 85vh;
		display: flex;
		flex-direction: column;
	}
	.dialog-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: var(--space-lg) var(--space-xl);
		border-bottom: 1px solid var(--color-border-light);
	}
	.dialog-header h2 {
		font-size: var(--font-size-lg);
		font-weight: 600;
	}
	.table-badge {
		font-family: var(--font-mono);
		color: var(--color-primary);
	}
	.close-btn {
		padding: var(--space-xs);
		border-radius: var(--radius-sm);
		color: var(--color-text-secondary);
	}
	.close-btn:hover { background: var(--color-bg-hover); }
	.dialog-body {
		padding: var(--space-xl);
		overflow-y: auto;
		flex: 1;
		display: flex;
		flex-direction: column;
		gap: var(--space-md);
	}
	.loading {
		padding: var(--space-xl);
		text-align: center;
		color: var(--color-text-muted);
	}
	.fields-list {
		display: flex;
		flex-direction: column;
		gap: var(--space-md);
	}
	.field-group {
		display: flex;
		flex-direction: column;
		gap: var(--space-xs);
	}
	.field-group label {
		display: flex;
		align-items: baseline;
		gap: var(--space-sm);
	}
	.field-label {
		font-size: var(--font-size-sm);
		font-weight: 600;
		font-family: var(--font-mono);
	}
	.field-type-hint {
		font-size: var(--font-size-xs);
		color: var(--color-text-muted);
	}
	.field-group input,
	.field-group select,
	.field-group textarea {
		padding: 7px 10px;
		border: 1px solid var(--color-border);
		border-radius: var(--radius-md);
		background: var(--color-bg);
		font-size: var(--font-size-sm);
		outline: none;
		resize: vertical;
	}
	.field-group input:focus,
	.field-group select:focus,
	.field-group textarea:focus {
		border-color: var(--color-primary);
	}
	.mono {
		font-family: var(--font-mono);
		font-size: var(--font-size-xs);
	}
	.sql-preview {
		border: 1px solid var(--color-border-light);
		border-radius: var(--radius-md);
		overflow: hidden;
	}
	.sql-preview-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: var(--space-xs) var(--space-md);
		background: var(--color-bg-tertiary);
		border-bottom: 1px solid var(--color-border-light);
	}
	.sql-preview-label {
		font-size: var(--font-size-xs);
		font-weight: 600;
		color: var(--color-text-secondary);
		text-transform: uppercase;
		letter-spacing: 0.3px;
	}
	.copy-btn {
		font-size: var(--font-size-xs);
		color: var(--color-primary);
		padding: 2px 8px;
		border-radius: var(--radius-sm);
	}
	.copy-btn:hover { background: var(--color-primary-subtle); }
	.sql-preview-code {
		padding: var(--space-md);
		margin: 0;
		font-family: var(--font-mono);
		font-size: var(--font-size-sm);
		line-height: 1.5;
		white-space: pre-wrap;
		word-break: break-word;
	}
	.success-msg {
		padding: var(--space-sm) var(--space-md);
		background: #f0f7ec;
		border: 1px solid #c8dfc0;
		border-radius: var(--radius-md);
		color: var(--color-success);
		font-size: var(--font-size-sm);
		font-weight: 500;
	}
	.dialog-error {
		padding: var(--space-sm) var(--space-md);
		background: #fdf0ef;
		border: 1px solid #f5c6c0;
		border-radius: var(--radius-md);
		color: var(--color-error);
		font-size: var(--font-size-sm);
		font-family: var(--font-mono);
	}
	.dialog-footer {
		display: flex;
		justify-content: flex-end;
		gap: var(--space-sm);
		padding: var(--space-lg) var(--space-xl);
		border-top: 1px solid var(--color-border-light);
	}
	.btn {
		padding: 7px 16px;
		border-radius: var(--radius-md);
		font-size: var(--font-size-sm);
		font-weight: 500;
	}
	.btn-ghost { color: var(--color-text-secondary); }
	.btn-ghost:hover { background: var(--color-bg-hover); }
	.btn-primary { background: var(--color-primary); color: white; }
	.btn-primary:hover:not(:disabled) { background: var(--color-primary-hover); }
	.btn-primary:disabled { opacity: 0.5; cursor: not-allowed; }
</style>
