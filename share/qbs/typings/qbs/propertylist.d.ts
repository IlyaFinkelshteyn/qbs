declare class PropertyList {
    constructor();
    clear(): void;
    isEmpty(): boolean;
    format(): string;
    readFromFile(filePath: string): void;
    readFromJSON(obj: any): void;
    readFromString(input: string): void;
    toJSON(): any;
    toJSONString(style?: string): string;
    toObject(): any;
    toString(format: string): string;
    toXMLString(): string;
    writeToFile(filePath: string, format: string): void;
}
