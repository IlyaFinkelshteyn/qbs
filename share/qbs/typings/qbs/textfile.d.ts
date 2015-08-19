declare enum OpenMode {
	ReadOnly,
	WriteOnly,
	ReadWrite
}

declare class TextFile {
	ReadOnly: OpenMode;
	WriteOnly: OpenMode;
	ReadWrite: OpenMode;
	
	constructor(filePath: string, openMode: OpenMode);
	atEof(): boolean;
	close(): void;
	readAll(): string;
	readLine(): string;
	setCodec(codec: string): void;
	truncate(): void;
	write(data: string): void;
	writeLine(data: string): void;
}
