<script lang="ts">
	import type { Tab } from '../../lib/types';

	let { tabs, activeTabId, onSelectTab, onCloseTab }: {
		tabs: Tab[];
		activeTabId: string;
		onSelectTab: (id: string) => void;
		onCloseTab: (id: string) => void;
	} = $props();
</script>

<div class="tabbar">
	{#each tabs as tab (tab.id)}
		<button
			class="tab"
			class:active={activeTabId === tab.id}
			onclick={() => onSelectTab(tab.id)}
		>
			<span class="tab-label">{tab.label}</span>
			</button>
		{#if tabs.length > 1}
			<span
				class="tab-close"
				role="button"
				tabindex="0"
				onclick={(e) => { e.stopPropagation(); onCloseTab(tab.id); }}
				onkeydown={(e) => { if (e.key === 'Enter') { e.stopPropagation(); onCloseTab(tab.id); } }}
				title="Close"
			>
				<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
					<line x1="18" y1="6" x2="6" y2="18"></line>
					<line x1="6" y1="6" x2="18" y2="18"></line>
				</svg>
			</span>
		{/if}
	{/each}
</div>

<style>
	.tabbar {
		display: flex;
		background: var(--color-bg-secondary);
		border-bottom: 1px solid var(--color-border);
		overflow-x: auto;
		flex-shrink: 0;
	}
	.tab {
		display: flex;
		align-items: center;
		gap: var(--space-xs);
		padding: var(--space-sm) var(--space-lg);
		font-size: var(--font-size-sm);
		color: var(--color-text-secondary);
		border-bottom: 2px solid transparent;
		white-space: nowrap;
		transition: color 0.15s, border-color 0.15s;
	}
	.tab:hover {
		color: var(--color-text);
		background: var(--color-bg-hover);
	}
	.tab.active {
		color: var(--color-primary);
		border-bottom-color: var(--color-primary);
		background: var(--color-bg);
	}
	.tab-close {
		padding: 2px;
		border-radius: var(--radius-sm);
		color: var(--color-text-muted);
		display: flex;
		align-items: center;
	}
	.tab-close:hover {
		background: var(--color-bg-hover);
		color: var(--color-text);
	}
</style>
