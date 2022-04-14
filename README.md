# MPEG-ts parser
The program parses MPEG-ts file and print information about the elementary streams based on PAT/PMT.

## Build
Run command:

```bash
make parser
```

And pass the file as argument with option `-f`:

```bash
./parser -f your_file.ts
```

Or pass the multicast group as argument with option `-m`:

```bash
./parser -m 225.1.1.1:5000
```
