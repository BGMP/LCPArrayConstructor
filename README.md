# LCP Array Constructor
A C program that constructs LCP (Longest Common Prefix) arrays from text files and outputs them as binary files
compatible with DACs (Directly Addressable Codes).

## What it does
This program takes any text file and:

1. **Builds a suffix array** - sorts all suffixes of the text lexicographically
2. **Computes the LCP array** - calculates how many characters each suffix shares with the previous one
3. **Outputs a binary file** - saves the LCP values as 32-bit integers ready for compression algorithms like DACs

The LCP array is essential for testing variable-length encoding schemes on realistic integer sequences where most values
are small but some can be very large.

## Build Instructions
### Prerequisites
  * CMake 3.27 or higher
  * C++ compiler with C++17 support (GCC, Clang, or MSVC)

### Building
```sh
# Clone or download the project
cd LCPArrayConstructor

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Or on Windows with Visual Studio
cmake ..
cmake --build . --config Release
```

## Usage
```sh
./LCPArrayGenerator <input_text_file> <output_binary_file>
```

### Options
  * -l, --little-endian - Output in little-endian format
  * -b, --big-endian - Output in big-endian format
  * -h, --help - Show help message with usage examples
