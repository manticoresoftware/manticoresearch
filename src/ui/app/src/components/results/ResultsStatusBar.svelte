<script lang="ts">
	let { rowCount = 0, total = 0, duration = 0, warning = '' }: {
		rowCount?: number;
		total?: number;
		duration?: number;
		warning?: string;
	} = $props();

	function formatDuration(ms: number): string {
		if (ms < 1) return '<1ms';
		if (ms < 1000) return `${Math.round(ms)}ms`;
		return `${(ms / 1000).toFixed(3)}s`;
	}
</script>

<div class="status-bar">
	<div class="status-left">
		<span class="status-item">Rows: <strong>{rowCount}</strong></span>
		{#if total > rowCount}
			<span class="status-item">of {total} total</span>
		{/if}
		{#if duration > 0}
			<span class="status-item">Time: <strong>{formatDuration(duration)}</strong></span>
		{/if}
	</div>
	{#if warning}
		<div class="status-warning">
			<svg width="12" height="12" viewBox="0 0 24 24" fill="currentColor">
				<path d="M12 2L1 21h22L12 2zm0 4l7.53 13H4.47L12 6zm-1 5v4h2v-4h-2zm0 6v2h2v-2h-2z"/>
			</svg>
			{warning}
		</div>
	{/if}
</div>

<style>
	.status-bar {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: var(--space-xs) var(--space-md);
		font-size: var(--font-size-xs);
		color: var(--color-text-secondary);
		background: var(--color-bg-secondary);
		border-top: 1px solid var(--color-border-light);
		border-radius: 0 0 var(--radius-md) var(--radius-md);
		flex-shrink: 0;
	}
	.status-left {
		display: flex;
		gap: var(--space-md);
	}
	.status-item strong {
		color: var(--color-text);
	}
	.status-warning {
		display: flex;
		align-items: center;
		gap: 4px;
		color: var(--color-warning);
	}
</style>
