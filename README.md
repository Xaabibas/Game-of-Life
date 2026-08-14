# Game of Life

Simple terminal implementation of Conway's Game of Life written in C using ncurses.

The field is represented as a torus, so cells on opposite edges are considered neighbors.

## Features

- Interactive field editing
- Conway's Game of Life simulation
- Random field generation
- Configurable simulation delay
- Arrow keys and `h`, `j`, `k`, `l` for navigation

## Requirements

- C compiler
- GNU Make
- ncurses
- pkg-config

## Build

```
make
```

To remove build file:
```
make clean
```

## Usage
```
./game_life [OPTIONS]
```

### Options

```
-h, --help          Show help message
-r, --rand <value> Fill the field randomly
-d, --delay <value>
                    Set delay between generations in milliseconds
```

For example:
```
./game_life --rand 30 --delay 150
```

## License
This project was created for educational purposes.
