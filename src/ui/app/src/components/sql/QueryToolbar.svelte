<script lang="ts">
	let { onExecute, onClear, executing = false, onToggleHistory }: {
		onExecute: () => void;
		onClear: () => void;
		executing?: boolean;
		onToggleHistory?: () => void;
	} = $props();
</script>

<div class="toolbar">
	<div class="toolbar-left">
		<button class="btn btn-primary" onclick={onExecute} disabled={executing}>
			<svg width="14" height="14" viewBox="0 0 24 24" fill="currentColor">
				<polygon points="5 3 19 12 5 21 5 3"></polygon>
			</svg>
			{executing ? 'Running...' : 'Run'}
			<kbd>Ctrl+Enter</kbd>
		</button>
		{#if onToggleHistory}
			<button class="btn btn-ghost" onclick={onToggleHistory}>
				<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
					<circle cx="12" cy="12" r="10"></circle>
					<polyline points="12 6 12 12 16 14"></polyline>
				</svg>
				History
			</button>
		{/if}
		<button class="btn btn-ghost" onclick={onClear}>Clear</button>
	</div>
</div>

<style>
	.toolbar {
		display: flex;
		align-items: center;
		padding: var(--space-sm) 0;
		flex-shrink: 0;
	}
	.toolbar-left {
		display: flex;
		gap: var(--space-sm);
	}
	.btn {
		display: inline-flex;
		align-items: center;
		gap: 6px;
		padding: 5px 12px;
		border-radius: var(--radius-md);
		font-size: var(--font-size-sm);
		font-weight: 500;
		transition: background 0.15s, color 0.15s;
	}
	.btn-primary {
		background: var(--color-primary);
		color: white;
	}
	.btn-primary:hover:not(:disabled) {
		background: var(--color-primary-hover);
	}
	.btn-primary:disabled {
		opacity: 0.6;
		cursor: not-allowed;
	}
	.btn-ghost {
		color: var(--color-text-secondary);
	}
	.btn-ghost:hover {
		background: var(--color-bg-hover);
		color: var(--color-text);
	}
	kbd {
		font-family: var(--font-sans);
		font-size: var(--font-size-xs);
		padding: 1px 5px;
		background: rgba(255,255,255,0.2);
		border-radius: 3px;
		opacity: 0.8;
	}
</style>
