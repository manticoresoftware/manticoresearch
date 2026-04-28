<script lang="ts">
	import { executeSQL } from '../../lib/api';
	import { refreshTables } from '../../lib/stores/tables.svelte';

	let { open = false, onClose }: {
		open: boolean;
		onClose: () => void;
	} = $props();

	const FIELD_TYPES = [
		{ value: 'text', label: 'text', desc: 'Full-text searchable' },
		{ value: 'string', label: 'string', desc: 'Stored string, filterable' },
		{ value: 'integer', label: 'integer', desc: '32-bit signed integer' },
		{ value: 'bigint', label: 'bigint', desc: '64-bit signed integer' },
		{ value: 'float', label: 'float', desc: 'Single-precision float' },
		{ value: 'bool', label: 'bool', desc: 'Boolean (0 or 1)' },
		{ value: 'json', label: 'json', desc: 'JSON object' },
		{ value: 'timestamp', label: 'timestamp', desc: 'Unix timestamp' },
		{ value: 'multi', label: 'multi', desc: 'Multi-value 32-bit integers' },
		{ value: 'multi64', label: 'multi64', desc: 'Multi-value 64-bit integers' },
		{ value: 'float_vector', label: 'float_vector', desc: 'KNN vector' },
	];

	const TABLE_TYPES = ['rt', 'percolate'];

	const SIMILARITY_TYPES = ['L2', 'cosine', 'IP'];

	interface Field {
		name: string;
		type: string;
		knnType: string;
		similarity: string;
		modelName: string;
		from: string;
	}

	let tableName = $state('');
	let tableType = $state('rt');
	let fields = $state<Field[]>([
		{ name: '', type: 'text', knnType: 'hnsw', similarity: 'L2', modelName: '', from: '' },
	]);
	let executing = $state(false);
	let error = $state<string | null>(null);

	function addField() {
		fields = [...fields, { name: '', type: 'text', knnType: 'hnsw', similarity: 'L2', modelName: '', from: '' }];
	}

	function removeField(idx: number) {
		fields = fields.filter((_, i) => i !== idx);
	}

	function buildSQL(): string {
		if (!tableName.trim() || fields.length === 0) return '';

		const defs = fields
			.filter(f => f.name.trim())
			.map(f => {
				let def = `${f.name} ${f.type}`;
				if (f.type === 'float_vector') {
					def += ` knn_type='${f.knnType}' hnsw_similarity='${f.similarity}'`;
					if (f.modelName.trim()) {
						def += ` model_name='${f.modelName}'`;
						if (f.from.trim()) {
							def += ` from='${f.from}'`;
						}
					}
				}
				return def;
			});

		if (defs.length === 0) return '';
		return `CREATE TABLE ${tableName} (\n  ${defs.join(',\n  ')}\n) type='${tableType}'`;
	}

	let generatedSQL = $derived(buildSQL());

	async function handleCreate() {
		if (!generatedSQL) return;
		executing = true;
		error = null;
		try {
			await executeSQL(generatedSQL);
			await refreshTables();
			resetForm();
			onClose();
		} catch (e) {
			error = e instanceof Error ? e.message : String(e);
		} finally {
			executing = false;
		}
	}

	function resetForm() {
		tableName = '';
		tableType = 'rt';
		fields = [{ name: '', type: 'text', knnType: 'hnsw', similarity: 'L2', modelName: '', from: '' }];
		error = null;
	}

	function handleClose() {
		resetForm();
		onClose();
	}

	function copySQL() {
		if (generatedSQL) navigator.clipboard.writeText(generatedSQL);
	}
</script>

{#if open}
<div class="overlay" onclick={handleClose} onkeydown={(e) => e.key === 'Escape' && handleClose()} role="dialog" tabindex="-1">
	<div class="dialog" onclick={(e) => e.stopPropagation()} role="document">
		<div class="dialog-header">
			<h2>Create Table</h2>
			<button class="close-btn" onclick={handleClose} title="Close">
				<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<line x1="18" y1="6" x2="6" y2="18"></line>
					<line x1="6" y1="6" x2="18" y2="18"></line>
				</svg>
			</button>
		</div>

		<div class="dialog-body">
			<div class="form-row">
				<div class="form-group flex-1">
					<label for="table-name">Table name</label>
					<input id="table-name" type="text" bind:value={tableName} placeholder="my_table" autocomplete="off" />
				</div>
				<div class="form-group">
					<label for="table-type">Type</label>
					<select id="table-type" bind:value={tableType}>
						{#each TABLE_TYPES as t}
							<option value={t}>{t}</option>
						{/each}
					</select>
				</div>
			</div>

			<div class="fields-section">
				<div class="fields-header">
					<span class="fields-label">Fields</span>
					<button class="add-field-btn" onclick={addField}>
						<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
							<line x1="12" y1="5" x2="12" y2="19"></line>
							<line x1="5" y1="12" x2="19" y2="12"></line>
						</svg>
						Add field
					</button>
				</div>

				<div class="fields-list">
					{#each fields as field, idx (idx)}
						<div class="field-row">
							<input
								class="field-name"
								type="text"
								bind:value={field.name}
								placeholder="field_name"
								autocomplete="off"
							/>
							<select class="field-type" bind:value={field.type}>
								{#each FIELD_TYPES as ft}
									<option value={ft.value} title={ft.desc}>{ft.label}</option>
								{/each}
							</select>

							{#if field.type === 'float_vector'}
								<select class="field-extra" bind:value={field.similarity} title="Similarity">
									{#each SIMILARITY_TYPES as s}
										<option value={s}>{s}</option>
									{/each}
								</select>
								<input
									class="field-model"
									type="text"
									bind:value={field.modelName}
									placeholder="model_name (optional)"
								/>
								{#if field.modelName.trim()}
									<input
										class="field-from"
										type="text"
										bind:value={field.from}
										placeholder="from field"
									/>
								{/if}
							{/if}

							<button class="remove-field-btn" onclick={() => removeField(idx)} title="Remove field" disabled={fields.length <= 1}>
								<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
									<line x1="18" y1="6" x2="6" y2="18"></line>
									<line x1="6" y1="6" x2="18" y2="18"></line>
								</svg>
							</button>
						</div>
					{/each}
				</div>
			</div>

			{#if generatedSQL}
				<div class="sql-preview">
					<div class="sql-preview-header">
						<span class="sql-preview-label">Generated SQL</span>
						<button class="copy-btn" onclick={copySQL} title="Copy to clipboard">Copy</button>
					</div>
					<pre class="sql-preview-code">{generatedSQL}</pre>
				</div>
			{/if}

			{#if error}
				<div class="dialog-error">{error}</div>
			{/if}
		</div>

		<div class="dialog-footer">
			<button class="btn btn-ghost" onclick={handleClose}>Cancel</button>
			<button class="btn btn-primary" onclick={handleCreate} disabled={!generatedSQL || executing}>
				{executing ? 'Creating...' : 'Create Table'}
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
		width: 640px;
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
		font-size: var(--font-size-xl);
		font-weight: 600;
	}
	.close-btn {
		padding: var(--space-xs);
		border-radius: var(--radius-sm);
		color: var(--color-text-secondary);
	}
	.close-btn:hover {
		background: var(--color-bg-hover);
	}
	.dialog-body {
		padding: var(--space-xl);
		overflow-y: auto;
		flex: 1;
		display: flex;
		flex-direction: column;
		gap: var(--space-lg);
	}
	.form-row {
		display: flex;
		gap: var(--space-md);
	}
	.flex-1 { flex: 1; }
	.form-group {
		display: flex;
		flex-direction: column;
		gap: var(--space-xs);
	}
	.form-group label {
		font-size: var(--font-size-xs);
		font-weight: 600;
		text-transform: uppercase;
		letter-spacing: 0.3px;
		color: var(--color-text-secondary);
	}
	.form-group input,
	.form-group select {
		padding: 7px 10px;
		border: 1px solid var(--color-border);
		border-radius: var(--radius-md);
		background: var(--color-bg);
		font-size: var(--font-size-md);
		outline: none;
	}
	.form-group input:focus,
	.form-group select:focus {
		border-color: var(--color-primary);
	}
	.fields-section {
		display: flex;
		flex-direction: column;
		gap: var(--space-sm);
	}
	.fields-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
	}
	.fields-label {
		font-size: var(--font-size-xs);
		font-weight: 600;
		text-transform: uppercase;
		letter-spacing: 0.3px;
		color: var(--color-text-secondary);
	}
	.add-field-btn {
		display: inline-flex;
		align-items: center;
		gap: 4px;
		padding: 4px 10px;
		font-size: var(--font-size-xs);
		color: var(--color-primary);
		border: 1px solid var(--color-primary-light);
		border-radius: var(--radius-md);
	}
	.add-field-btn:hover {
		background: var(--color-primary-subtle);
	}
	.fields-list {
		display: flex;
		flex-direction: column;
		gap: var(--space-sm);
	}
	.field-row {
		display: flex;
		align-items: center;
		gap: var(--space-sm);
		flex-wrap: wrap;
	}
	.field-name, .field-type, .field-extra {
		height: 34px;
		padding: 0 10px;
		border: 1px solid var(--color-border);
		border-radius: var(--radius-md);
		font-size: var(--font-size-sm);
		background: var(--color-bg);
		outline: none;
	}
	.field-name {
		flex: 1;
		min-width: 120px;
		font-family: var(--font-mono);
	}
	.field-type {
		width: 130px;
	}
	.field-extra {
		width: 80px;
	}
	.field-name:focus, .field-type:focus, .field-extra:focus {
		border-color: var(--color-primary);
	}
	.field-model, .field-from {
		flex: 1;
		min-width: 100px;
		padding: 6px 10px;
		border: 1px solid var(--color-border);
		border-radius: var(--radius-md);
		font-size: var(--font-size-xs);
		font-family: var(--font-mono);
		background: var(--color-bg);
		outline: none;
	}
	.field-model:focus, .field-from:focus { border-color: var(--color-primary); }
	.remove-field-btn {
		padding: 4px;
		border-radius: var(--radius-sm);
		color: var(--color-text-muted);
		flex-shrink: 0;
	}
	.remove-field-btn:hover:not(:disabled) {
		color: var(--color-error);
		background: #fdf0ef;
	}
	.remove-field-btn:disabled {
		opacity: 0.3;
		cursor: not-allowed;
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
		color: var(--color-text);
		white-space: pre-wrap;
		word-break: break-word;
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
	.btn-ghost {
		color: var(--color-text-secondary);
	}
	.btn-ghost:hover { background: var(--color-bg-hover); }
	.btn-primary {
		background: var(--color-primary);
		color: white;
	}
	.btn-primary:hover:not(:disabled) { background: var(--color-primary-hover); }
	.btn-primary:disabled { opacity: 0.5; cursor: not-allowed; }
</style>
