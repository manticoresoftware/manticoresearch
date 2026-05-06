import { fetchTables } from '../api';
import type { TableInfo } from '../types';

let tables = $state<TableInfo[]>([]);
let loading = $state(false);
let error = $state<string | null>(null);

export function getTablesState() {
	return {
		get tables() { return tables; },
		get loading() { return loading; },
		get error() { return error; },
	};
}

export async function refreshTables() {
	loading = true;
	error = null;
	try {
		tables = await fetchTables();
	} catch (e) {
		error = e instanceof Error ? e.message : String(e);
	} finally {
		loading = false;
	}
}
