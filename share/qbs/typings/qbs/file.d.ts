declare module File2 {
    function copy(sourceFilePath: string, targetFilePath: string): boolean;
    function directoryEntries(filePath: string): string[];
    function exists(filePath: string): boolean;
    function lastModified(filePath: string): number;
    function remove(filePath: string): boolean;
}
