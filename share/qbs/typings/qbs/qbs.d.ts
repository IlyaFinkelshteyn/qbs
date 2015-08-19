interface Array<T> {
    contains(obj: T): boolean;
    uniqueConcat(other: T[]): T[];
}

interface String {
    contains(s: string): boolean;
    startsWith(s: string): boolean;
    endsWith(s: string): boolean;
}

declare module qbs {
    function getEnv(key: string): string;
    function getHash(key: string): string;
    function getNativeSetting(key: string, value: string): string;
    function rfc1034Identifier(str: string): string;
}

declare class AbstractCommand {
    description: string;
    highlight: string;
    silent: boolean;

    [key: string]: any;
}

declare class Command extends AbstractCommand {
    constructor(command: string, args: string[]);
    arguments: string[];
    environment: string[];
    maxExitCode: number;
    program: string;
    responseFileThreshold: number;
    responseFileUsagePrefix: string;
    stderrFilterFunction: (output: string) => string;
    stdoutFilterFunction: (output: string) => string;
    workingDirectory: string;
}

declare class JavaScriptCommand extends AbstractCommand {
    constructor();
    sourceCode: () => void;
}

interface Module {
    moduleName: string;
    moduleProperty<T>(moduleName: string, propertyName: string): T;
    moduleProperties<T>(moduleName: string, propertyName: string): T[];
}

interface Artifact extends Module {
    condition: boolean;
    baseDir: string;
    completeBaseName: string;
    fileName: string;
    filePath: string;
    fileTags: string[];
    alwaysUpdated: boolean;
}

interface ArtifactMap {
    [fileTag: string]: Artifact[];

    // Known file tags, for increased type safety over string indexing
    application: Artifact[];
    assetcatalog: Artifact[];
    compiled_assetcatalog: Artifact[];
    compiled_ibdoc_main: Artifact[];
    debuginfo: Artifact[];
    debuginfo_bundle: Artifact[];
    dynamiclibrary: Artifact[];
    dynamiclibrary_copy: Artifact[];
    dynamiclibrary_import: Artifact[];
    infoplist: Artifact[];
    loadablemodule: Artifact[];
    obj: Artifact[];
    partial_infoplist: Artifact[];
    staticlibrary: Artifact[];
    typescript: Artifact[];
    typescript_declaration: Artifact[];
}

interface Product extends Module {
    condition: boolean;
    name: string;
    profiles: string[];
    type: string[];
    targetName: string;
    destinationDirectory: string;
    files: string[];
    excludeFiles: string[];
    consoleApplication: boolean;
    qbsSearchPaths: string[];
    version: string;

    buildDirectory: string;
    profile: string;
    sourceDirectory: string;
}

interface Project {
    sourceDirectory: string;
    buildDirectory: string;
}

declare function print(...args: any[]): void;
declare function getEnv(name: string): string;
declare function putEnv(name: string, value: string): void;

declare function loadExtension<T>(extensionName: string): T;
declare function loadFile<T>(filePath: string): T;
