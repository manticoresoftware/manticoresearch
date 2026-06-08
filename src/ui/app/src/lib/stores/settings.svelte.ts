const STORAGE_KEY = 'manticore-ui-settings';

interface Settings {
	editorFontSize: number;
	resultLimit: number;
}

const defaults: Settings = {
	editorFontSize: 14,
	resultLimit: 20,
};

function load(): Settings {
	try {
		const raw = localStorage.getItem(STORAGE_KEY);
		if (raw) return { ...defaults, ...JSON.parse(raw) };
	} catch {}
	return { ...defaults };
}

let settings = $state<Settings>(load());

export function getSettings() {
	return {
		get editorFontSize() { return settings.editorFontSize; },
		get resultLimit() { return settings.resultLimit; },
	};
}

export function updateSettings(patch: Partial<Settings>) {
	settings = { ...settings, ...patch };
	localStorage.setItem(STORAGE_KEY, JSON.stringify(settings));
}
