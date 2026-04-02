import type { QueryResult, ColumnDef, TableInfo } from './types';

export class QueryError extends Error {
	constructor(message: string) {
		super(message);
		this.name = 'QueryError';
	}
}

export async function executeSQL(query: string): Promise<QueryResult> {
	const resp = await fetch('/cli_json', {
		method: 'POST',
		body: query,
	});

	if (!resp.ok) {
		const text = await resp.text();
		throw new QueryError(`HTTP ${resp.status}: ${text}`);
	}

	const json = await resp.json();

	if (!Array.isArray(json) || json.length === 0) {
		return { columns: [], data: [], total: 0, error: '', warning: '' };
	}

	const result = json[0];

	if (result.error && result.error.length > 0) {
		throw new QueryError(result.error);
	}

	const columns: ColumnDef[] = [];
	if (result.columns) {
		for (const col of result.columns) {
			const name = Object.keys(col)[0];
			const type = col[name].type || '';
			columns.push({ name, type });
		}
	}

	return {
		columns,
		data: result.data || [],
		total: result.total ?? result.data?.length ?? 0,
		error: result.error || '',
		warning: result.warning || '',
	};
}

export async function fetchTables(): Promise<TableInfo[]> {
	const result = await executeSQL('SHOW TABLES');
	const tables = result.data.map((row: Record<string, unknown>) => ({
		name: (row['Table'] || row['table'] || '') as string,
		type: (row['Type'] || row['type'] || '') as string,
	}));

	// Fetch row counts in parallel
	const withCounts = await Promise.all(
		tables.map(async (t) => {
			try {
				const status = await executeSQL(`SHOW TABLE ${t.name} STATUS LIKE 'indexed_documents'`);
				const countRow = status.data[0];
				const val = countRow?.['Value'] ?? countRow?.['value'];
				return { ...t, count: val !== undefined ? Number(val) : undefined };
			} catch {
				return t;
			}
		})
	);

	return withCounts;
}

export async function describeTable(name: string): Promise<QueryResult> {
	return executeSQL(`DESCRIBE ${name}`);
}

export async function tableStatus(name: string): Promise<QueryResult> {
	return executeSQL(`SHOW TABLE ${name} STATUS`);
}

export async function serverStatus(): Promise<QueryResult> {
	return executeSQL('SHOW STATUS');
}

export async function serverVariables(): Promise<QueryResult> {
	return executeSQL('SHOW VARIABLES');
}

export async function callKeywords(text: string, table: string): Promise<QueryResult> {
	const escaped = text.replace(/'/g, "\\'");
	return executeSQL(`CALL KEYWORDS('${escaped}', '${table}', 1)`);
}
