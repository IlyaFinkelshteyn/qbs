import qbs

// $ g++ 'i like spaces.cpp' '-DSPACES="!have \\fun\x5c!\n"' '-DSPICES=%T% # && $$ 1>&2 '\''\n'\''\n' '-DSLICES=(42>24)' && ./a.out
// SPACES=!have \fun\!
// SPICES=%T% # && $$ 1>&2 '\n'
// SLICES=1

Project {
Application {
    targetName: "i like spaces"

    Depends {
        name: "cpp"
    }

    cpp.defines: base.concat([
        "SPACES=\"!have \\\\fun\\x5c!\\n\"",
        "SPICES=%T% # && $$ 1>&2 '\\n'\\n",
        "SLICES=(42>24)"
    ]);

    files: ["i like spaces.cpp"]
}
}
