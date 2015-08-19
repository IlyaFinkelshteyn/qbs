export declare module io.qt.qbs {
    class Artifact {
        filePath: string;
        fileTags: string[];
    }
    module tools {
        module utils {
            function artifactFromFilePath(filePath: string): Artifact;
        }
        function compile(commandLineArguments: string[]): qbs.Artifact[];
        function TypeScriptCompilerScannerToolMain(): void;
    }
}
