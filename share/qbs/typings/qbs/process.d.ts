declare class Process {
    constructor();
    close(): void;
    exec(filePath: string, args: string[], throwOnError: boolean): number;
    exitCode(): number;
    getEnv(varName: string): string;
    kill(): void;
    readLine(): string;
    readStdErr(): string;
    readStdOut(): string;
    setEnv(varName: string, varValue: string): string;
    setWorkingDirectory(path: string): void;
    start(filePath: string, args: string[]): boolean;
    terminate(): void;
    waitForFinished(timeout?: number): boolean;
    workingDirectory(): string;
    write(data: string): void;
    writeLine(data: string): void;
}
