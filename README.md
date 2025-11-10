# Fluid_Physics_Simulation

YouTube link for a demonstration video: [https://www.youtube.com/watch?v=iZVx1ZC-qsQ](https://www.youtube.com/watch?v=iZVx1ZC-qsQ)

# Command Line Options

The command line options can be displayed with `-?` or `--help`:

```
-?              Display command line options and quit.
--help          Alias for -?.
-benchmark      Run simulation for 3 minutes (~10,800 frames @ 60fps), render first frame at frame number 7,200.
-benchfast      Run simulation for 10 seconds (~600 frames @ 60fps), render first frame at frame number 300.
-render #       Don't render until specified frame number. -1 is never render. (Default 0).
-time   #.##    Run simulation for specified seconds.
-v              Verbose mode off (default).
+v              Verbose mode on.
-V              Display version and quit.
--version       Alias for -V.
-vsync          VSync off.
+vsync          VSync on (default).
```

# Benchmarking

There are two benchmark modes:

* `-benchmark`
* `-benchfast`

| Command | Rendering starts at frame # | Simulation ends at time |
|:-------------|------:|-----------:|
| `-benchfast` |   300 | 10 seconds |
| `-benchmark` | 7,200 |  3 minutes |
