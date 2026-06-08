import { keymap, highlightSpecialChars, drawSelection, highlightActiveLine, lineNumbers, highlightActiveLineGutter } from '@codemirror/view';
import { EditorState, type Extension } from '@codemirror/state';
import { defaultHighlightStyle, syntaxHighlighting, indentOnInput, bracketMatching } from '@codemirror/language';
import { defaultKeymap, history, historyKeymap } from '@codemirror/commands';
import { closeBrackets, closeBracketsKeymap, autocompletion, completionKeymap } from '@codemirror/autocomplete';
import { searchKeymap, highlightSelectionMatches } from '@codemirror/search';
import { sql, MySQL } from '@codemirror/lang-sql';

export function createEditorExtensions(onExecute: () => void, tableNames: string[]): Extension[] {
	const schema: Record<string, string[]> = {};
	for (const t of tableNames) {
		schema[t] = [];
	}

	return [
		lineNumbers(),
		highlightActiveLineGutter(),
		highlightSpecialChars(),
		history(),
		drawSelection(),
		indentOnInput(),
		bracketMatching(),
		closeBrackets(),
		highlightActiveLine(),
		highlightSelectionMatches(),
		syntaxHighlighting(defaultHighlightStyle, { fallback: true }),
		sql({
			dialect: MySQL,
			upperCaseKeywords: true,
			schema,
		}),
		autocompletion({
			activateOnTyping: true,
		}),
		keymap.of([
			...completionKeymap,
			{
				key: 'Ctrl-Enter',
				mac: 'Cmd-Enter',
				run: () => { onExecute(); return true; },
			},
			...closeBracketsKeymap,
			...defaultKeymap,
			...searchKeymap,
			...historyKeymap,
		]),
		EditorState.tabSize.of(2),
	];
}
