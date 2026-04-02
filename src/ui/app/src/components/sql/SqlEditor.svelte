<script lang="ts">
	import { onMount } from 'svelte';
	import { EditorView } from '@codemirror/view';
	import { EditorState } from '@codemirror/state';
	import { createEditorExtensions } from '../../lib/codemirror/setup';

	let { onExecute, tableNames = [], initialValue = '', onReady }: {
		onExecute: (sql: string) => void;
		tableNames?: string[];
		initialValue?: string;
		onReady?: (api: { getValue: () => string; setValue: (v: string) => void }) => void;
	} = $props();

	let container: HTMLDivElement;
	let view: EditorView;

	function getValue(): string {
		if (!view) return '';
		const sel = view.state.selection.main;
		if (sel.from !== sel.to) {
			return view.state.sliceDoc(sel.from, sel.to);
		}
		return view.state.doc.toString();
	}

	function setValue(text: string) {
		if (!view) return;
		view.dispatch({
			changes: { from: 0, to: view.state.doc.length, insert: text },
		});
	}

	function handleExecute() {
		onExecute(getValue());
	}

	onMount(() => {
		const extensions = createEditorExtensions(handleExecute, tableNames);
		const state = EditorState.create({
			doc: initialValue,
			extensions,
		});
		view = new EditorView({ state, parent: container });
		onReady?.({ getValue, setValue });

		return () => view.destroy();
	});
</script>

<div class="editor-wrap" bind:this={container}></div>

<style>
	.editor-wrap {
		flex: 1;
		overflow: auto;
		border: 1px solid var(--color-border);
		border-radius: var(--radius-md);
		background: var(--color-bg);
	}
	.editor-wrap :global(.cm-editor) {
		height: 100%;
	}
	.editor-wrap :global(.cm-scroller) {
		font-family: var(--font-mono);
		font-size: 14px;
		line-height: 1.5;
	}
	.editor-wrap :global(.cm-focused) {
		outline: none;
	}
	.editor-wrap :global(.cm-gutters) {
		background: var(--color-bg-secondary);
		border-right: 1px solid var(--color-border-light);
		color: var(--color-text-muted);
	}
	.editor-wrap :global(.cm-activeLineGutter) {
		background: var(--color-bg-hover);
	}
	.editor-wrap :global(.cm-selectionBackground) {
		background: rgba(123, 140, 45, 0.25) !important;
	}
	.editor-wrap :global(.cm-focused .cm-selectionBackground) {
		background: rgba(123, 140, 45, 0.3) !important;
	}
	.editor-wrap :global(.cm-selectionMatch) {
		background: rgba(123, 140, 45, 0.15);
	}
	.editor-wrap :global(.cm-activeLine) {
		background: var(--color-primary-subtle);
	}
</style>
