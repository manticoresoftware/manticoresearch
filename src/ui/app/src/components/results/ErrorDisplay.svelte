<script lang="ts">
	let { message }: { message: string } = $props();
</script>

<div class="error-display">
	<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
		<circle cx="12" cy="12" r="10"></circle>
		<line x1="15" y1="9" x2="9" y2="15"></line>
		<line x1="9" y1="9" x2="15" y2="15"></line>
	</svg>
	<span class="error-text">{message}</span>
</div>

<style>
	.error-display {
		display: flex;
		align-items: flex-start;
		gap: var(--space-sm);
		padding: var(--space-md);
		background: #fdf0ef;
		border: 1px solid #f5c6c0;
		border-radius: var(--radius-md);
		color: var(--color-error);
		font-size: var(--font-size-sm);
		font-family: var(--font-mono);
	}
	.error-display svg {
		flex-shrink: 0;
		margin-top: 1px;
	}
	.error-text {
		white-space: pre-wrap;
		word-break: break-word;
	}
</style>
