interface clipboard {

	/**
	Write a string to the system clipboard. Returns true if the operation succeeded or false if an error occurred
	@param value - The string to write to the system clipboard
	**/
	write(value: string): boolean;

	/**
	Read the data in the system clipboard as a string. Returns null if clipboard is empty or an error occurred
	**/
	read(): string | null;
}

declare const $: clipboard;
export = $;
