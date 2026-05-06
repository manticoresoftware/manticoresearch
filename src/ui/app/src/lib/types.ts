export interface ColumnDef {
	name: string;
	type: string;
}

export interface QueryResult {
	columns: ColumnDef[];
	data: Record<string, unknown>[];
	total: number;
	error: string;
	warning: string;
}

export interface TableInfo {
	name: string;
	type: string;
	count?: number;
}

export interface HistoryEntry {
	query: string;
	timestamp: number;
	success: boolean;
	duration?: number;
}

export type TabId = 'sql' | 'table' | 'status';

export interface Tab {
	id: string;
	type: TabId;
	label: string;
	tableName?: string;
}
