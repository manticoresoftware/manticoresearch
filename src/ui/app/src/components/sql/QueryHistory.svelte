<script lang="ts">
	import type { HistoryEntry } from '../../lib/types';

	let { entries, onSelect, onClose }: {
		entries: HistoryEntry[];
		onSelect: (query: string) => void;
		onClose: () => void;
	} = $props();

	function formatTime(ts: number): string {
		return new Date(ts).toLocaleTimeString();
	}

	function formatDuration(ms?: number): string {
		if (ms === undefined) return '';
		if (ms < 1) return '<1ms';
		if (ms < 1000) return `${Math.round(ms)}ms`;
		return `${(ms / 1000).toFixed(2)}s`;
	}
</script>

<div class="history-panel">
	<div class="history-header">
		<span class="history-title">Query History</span>
		<button class="close-btn" onclick={onClose} title="Close">
			<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
				<line x1="18" y1="6" x2="6" y2="18"></line>
				<line x1="6" y1="6" x2="18" y2="18"></line>
			</svg>
		</button>
	</div>
	<div class="history-list">
		{#if entries.length === 0}
			<div class="history-empty">No queries yet</div>
		{:else}
			{#each entries as entry (entry.timestamp)}
				<button class="history-item" onclick={() => onSelect(entry.query)}>
					<code class="history-query">{entry.query.slice(0, 200)}</code>
					<span class="history-meta">
						<span class:success={entry.success} class:failed={!entry.success}>
							{entry.success ? 'OK' : 'ERR'}
						</span>
						{#if entry.duration !== undefined}
							<span class="history-duration">{formatDuration(entry.duration)}</span>
						{/if}
						<span class="history-time">{formatTime(entry.timestamp)}</span>
					</span>
				</button>
			{/each}
		{/if}
	</div>
</div>

<style>
	.history-panel {
		position: absolute;
		top: 0;
		right: 0;
		bottom: 0;
		width: 380px;
		background: var(--color-bg);
		border-left: 1px solid var(--color-border);
		box-shadow: var(--shadow-md);
		display: flex;
		flex-direction: column;
		z-index: 10;
	}
	.history-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: var(--space-md) var(--space-lg);
		border-bottom: 1px solid var(--color-border-light);
	}
	.history-title {
		font-weight: 600;
		font-size: var(--font-size-sm);
	}
	.close-btn {
		padding: var(--space-xs);
		border-radius: var(--radius-sm);
		color: var(--color-text-secondary);
	}
	.close-btn:hover {
		background: var(--color-bg-hover);
	}
	.history-list {
		overflow-y: auto;
		flex: 1;
	}
	.history-empty {
		padding: var(--space-xl);
		text-align: center;
		color: var(--color-text-muted);
		font-size: var(--font-size-sm);
	}
	.history-item {
		display: flex;
		flex-direction: column;
		gap: var(--space-xs);
		width: 100%;
		padding: var(--space-sm) var(--space-lg);
		text-align: left;
		border-bottom: 1px solid var(--color-border-light);
	}
	.history-item:hover {
		background: var(--color-bg-hover);
	}
	.history-query {
		font-family: var(--font-mono);
		font-size: var(--font-size-xs);
		color: var(--color-text);
		white-space: nowrap;
		overflow: hidden;
		text-overflow: ellipsis;
	}
	.history-meta {
		display: flex;
		gap: var(--space-sm);
		font-size: var(--font-size-xs);
		color: var(--color-text-muted);
	}
	.success { color: var(--color-success); }
	.failed { color: var(--color-error); }
</style>
