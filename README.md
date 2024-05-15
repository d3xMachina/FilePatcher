# FilePatcher

Software to apply patches with pattern matching to any binary file. Has a low memory footprint.

## How to use

- Download the release or compile with VS Studio 2022.
- Run FilePatcher.exe in a command prompt.

## Command Syntax

```
Usage: FilePatcher.exe <input file> <output file> <pattern> <replacement> <occurrence>
- Pattern and replacement should be hexadecimal strings without spaces, e.g., DEADBEEF BAADC0DE.
- Use ? for wildcards in the pattern and replacement, e.g., DE??BEEF BAADC0DE, or DE??BEEF BA??C0DE to keep the wildcard content unchanged.
- Occurrence should be 'all' or a number starting from 1.

Example : FilePatcher.exe myfile myfile_patched DE??BEEF BA??C0DE 1
```

## License

FilePatcher is available on Github under the MIT license.