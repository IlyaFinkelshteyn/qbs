/**
    * Given a list of file tags, returns the file tag (one of [c, cpp, objc, objcpp])
    * corresponding to the C-family language the file should be compiled as.
    *
    * If no such tag is found, undefined is returned. If more than one match is
    * found, an exception is thrown.
    */
export declare function fileTagForTargetLanguage(fileTags: string[]): any;
/**
    * Returns the name of a language-specific property given the file tag
    * for that property, and the base property name.
    *
    * If \a fileTag is undefined, the language-agnostic property name is returned.
    *
    * @param propertyName flags, platformFlags, precompiledHeader
    * @param fileTag c, cpp, objc, objcpp
    */
export declare function languagePropertyName(propertyName: string, fileTag: string): string;
export declare function moduleProperties<T>(config: Module, key: string, langFilter?: string): T[];
export declare function modulePropertiesFromArtifacts<T>(product: Module, artifacts: Artifact[], moduleName: string, propertyName: string, langFilter?: string): T[];
export declare function moduleProperty<T>(product: Module, propertyName: string, langFilter?: string): T;
/**
    * Returns roughly the same value as moduleProperty for a product, but ensures that all of the
    * given input artifacts share the same value of said property, as a sort of sanity check.
    *
    * This allows us to verify that users do not, for example, try to set different values on input
    * artifacts for which the value is input specific (not product specific), but which must be the
    * same for all inputs.
    */
export declare function modulePropertyFromArtifacts<T>(product: Module, artifacts: Artifact[], moduleName: string, propertyName: string, langFilter?: string): T;
export declare function dumpProperty(key: string, value: any, level: number): void;
export declare function traverseObject(obj: any, func: (key: string, value: any, level: number) => void, level: number): void;
export declare function dumpObject(obj: any, description: string): void;
export declare function concatAll(): any[];
export declare function allFileTags(fileTaggers: any): any[];
/**
    * Flattens an environment dictionary (string keys to arrays or strings)
    * into a string list containing items like \c key=value1:value2:value3
    */
export declare function flattenEnvironmentDictionary(dict: any, pathListSeparator: string): any[];
export declare class BlackboxOutputArtifactTracker {
    command: string;
    commandArgsFunction: (outputDirectory: string) => string[];
    commandEnvironmentFunction: (outputDirectory: string) => string[];
    processStdOutFunction: (stdout: string) => Artifact[];
    fileTaggers: {
        [key: string]: string[];
    };
    allowUntaggedFiles: boolean;
    hostOS: string[];
    artifacts(outputDirectory: string): any[];
    createArtifact(filePath: string): {
        filePath: string;
        fileTags: string[];
    };
    findFiles(dir: string): any;
    private fixArtifactPaths(artifacts, realBasePath, fakeBasePath);
}
