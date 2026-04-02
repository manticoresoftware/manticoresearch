<script lang="ts">
	import Header from './components/layout/Header.svelte';
	import Sidebar from './components/layout/Sidebar.svelte';
	import TabBar from './components/layout/TabBar.svelte';
	import SqlEditorTab from './components/sql/SqlEditorTab.svelte';
	import TableDetailTab from './components/tables/TableDetailTab.svelte';
	import ServerStatusTab from './components/status/ServerStatusTab.svelte';
	import CreateTableDialog from './components/dialogs/CreateTableDialog.svelte';
	import InsertRowDialog from './components/dialogs/InsertRowDialog.svelte';
	import type { Tab } from './lib/types';

	let connected = $state(true);
	let showCreateTable = $state(false);
	let showInsertRow = $state(false);
	let insertTableName = $state('');
	let tabs = $state<Tab[]>([
		{ id: 'sql-1', type: 'sql', label: 'SQL Editor' },
	]);
	let activeTabId = $state('sql-1');
	let selectedTable = $state('');
	let sqlInitialQuery = $state('');
	let tabCounter = $state(1);

	function openTableTab(name: string) {
		selectedTable = name;
		const existingTab = tabs.find(t => t.type === 'table' && t.tableName === name);
		if (existingTab) {
			activeTabId = existingTab.id;
			return;
		}
		const tab: Tab = { id: `table-${name}`, type: 'table', label: name, tableName: name };
		tabs = [...tabs, tab];
		activeTabId = tab.id;
	}

	function openSqlTab(query?: string) {
		tabCounter++;
		const tab: Tab = { id: `sql-${tabCounter}`, type: 'sql', label: 'SQL Editor' };
		tabs = [...tabs, tab];
		activeTabId = tab.id;
		if (query) sqlInitialQuery = query;
	}

	function openStatusTab() {
		const existing = tabs.find(t => t.type === 'status');
		if (existing) {
			activeTabId = existing.id;
			return;
		}
		const tab: Tab = { id: 'status', type: 'status', label: 'Server Status' };
		tabs = [...tabs, tab];
		activeTabId = tab.id;
	}

	function closeTab(id: string) {
		if (tabs.length <= 1) return;
		const idx = tabs.findIndex(t => t.id === id);
		tabs = tabs.filter(t => t.id !== id);
		if (activeTabId === id) {
			activeTabId = tabs[Math.min(idx, tabs.length - 1)].id;
		}
	}

	function handleOpenQuery(sql: string) {
		const sqlTab = tabs.find(t => t.type === 'sql');
		if (sqlTab) {
			activeTabId = sqlTab.id;
			sqlInitialQuery = sql;
		} else {
			openSqlTab(sql);
		}
	}

	function checkConnection() {
		fetch('/cli_json', { method: 'POST', body: 'SHOW STATUS LIKE \'uptime\'' })
			.then(r => { connected = r.ok; })
			.catch(() => { connected = false; });
	}

	$effect(() => {
		checkConnection();
		const interval = setInterval(checkConnection, 30000);
		return () => clearInterval(interval);
	});

	let activeTab = $derived(tabs.find(t => t.id === activeTabId));
</script>

<div class="app">
	<Header {connected} />
	<div class="body">
		<Sidebar onSelectTable={openTableTab} {selectedTable} onCreateTable={() => showCreateTable = true} />
		<div class="main">
			<div class="main-tabs">
				<TabBar {tabs} {activeTabId} onSelectTab={(id) => activeTabId = id} onCloseTab={closeTab} />
				<div class="main-tabs-actions">
					<button class="tab-action" onclick={() => openSqlTab()} title="New SQL tab">
						<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
							<line x1="12" y1="5" x2="12" y2="19"></line>
							<line x1="5" y1="12" x2="19" y2="12"></line>
						</svg>
					</button>
					<button class="tab-action" onclick={openStatusTab} title="Server Status">
						<svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
							<rect x="3" y="3" width="18" height="18" rx="2" ry="2"></rect>
							<line x1="3" y1="9" x2="21" y2="9"></line>
							<line x1="9" y1="21" x2="9" y2="9"></line>
						</svg>
					</button>
				</div>
			</div>
			<div class="content">
				{#if activeTab?.type === 'sql'}
					<SqlEditorTab initialQuery={sqlInitialQuery} />
				{:else if activeTab?.type === 'table' && activeTab.tableName}
					<TableDetailTab tableName={activeTab.tableName} onOpenQuery={handleOpenQuery} onInsertRow={(name) => { insertTableName = name; showInsertRow = true; }} />
				{:else if activeTab?.type === 'status'}
					<ServerStatusTab />
				{/if}
			</div>
		</div>
	</div>
</div>

<CreateTableDialog open={showCreateTable} onClose={() => showCreateTable = false} />
<InsertRowDialog open={showInsertRow} tableName={insertTableName} onClose={() => showInsertRow = false} />

<style>
	.app {
		height: 100%;
		display: flex;
		flex-direction: column;
	}
	.body {
		flex: 1;
		display: flex;
		overflow: hidden;
	}
	.main {
		flex: 1;
		display: flex;
		flex-direction: column;
		overflow: hidden;
		min-width: 0;
	}
	.main-tabs {
		display: flex;
		align-items: stretch;
		flex-shrink: 0;
	}
	.main-tabs :global(.tabbar) {
		flex: 1;
	}
	.main-tabs-actions {
		display: flex;
		align-items: center;
		padding: 0 var(--space-sm);
		gap: 2px;
		background: var(--color-bg-secondary);
		border-bottom: 1px solid var(--color-border);
	}
	.tab-action {
		padding: var(--space-xs);
		border-radius: var(--radius-sm);
		color: var(--color-text-secondary);
		display: flex;
		align-items: center;
	}
	.tab-action:hover {
		background: var(--color-bg-hover);
		color: var(--color-text);
	}
	.content {
		flex: 1;
		display: flex;
		flex-direction: column;
		overflow: hidden;
	}
</style>
