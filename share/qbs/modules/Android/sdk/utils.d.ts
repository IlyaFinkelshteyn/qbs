export declare function sourceAndTargetFilePathsFromInfoFiles(inputs: ArtifactMap, product: Product, inputTag: string): {
    sourcePaths: string[];
    targetPaths: string[];
};
export declare function outputArtifactsFromInfoFiles(inputs: ArtifactMap, product: Product, inputTag: string, outputTag: string): Artifact[];
export declare function availableSdkPlatforms(sdkDir: string): any[];
export declare function availableBuildToolsVersions(sdkDir: any): any[];
