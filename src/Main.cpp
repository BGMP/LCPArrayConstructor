////////////////////////////////////////////////////////////////////////////////
//
// LCPArrayConstructor (Generic)
//
// Desc: Main.cpp
// Main program file.
//
// 07/06/2025 (BGM)
// File inception.
//
////////////////////////////////////////////////////////////////////////////////

#include "Main.h"

////////////////////////////////////////////////////////////////////////////////
// Macros:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Types:
////////////////////////////////////////////////////////////////////////////////

void CleanUpGlobals();
int* ConstructSuffixArray(char* input_text, int n);
int* ComputeLCPArray(char* text, int* suffix_array, int n);
char* ReadFile(const char* filename, long* file_size);
int WriteLCPToBinary(const char* filename, int* lcp_array, int n, endian_t endianness);
int main(int argc, char* argv[]);

////////////////////////////////////////////////////////////////////////////////
// Prototypes:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Globals:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Procedures:
////////////////////////////////////////////////////////////////////////////////

void CleanUpGlobals() {
    if (g_text) { free(g_text); g_text = NULL; }
    if (g_sa) { free(g_sa); g_sa = NULL; }
    if (g_sa2) { free(g_sa2); g_sa2 = NULL; }
    if (g_rank) { free(g_rank); g_rank = NULL; }
    if (g_c) { free(g_c); g_c = NULL; }
    if (g_lcp) { free(g_lcp); g_lcp = NULL; }
}

// Direct translation of Java suffix array construction
int* ConstructSuffixArray(char* input_text, int n) {
    int ALPHABET_SZ = 256;

    printf("  Allocating memory for suffix array construction...\n");

    // Allocate global arrays to avoid stack issues
    g_text = (int*)malloc(n * sizeof(int));
    g_sa = (int*)malloc(n * sizeof(int));
    g_sa2 = (int*)malloc(n * sizeof(int));
    g_rank = (int*)malloc(n * sizeof(int));

    if (!g_text || !g_sa || !g_sa2 || !g_rank) {
        printf("Error: Failed to allocate memory for suffix array construction\n");
        CleanUpGlobals();
        return NULL;
    }

    printf("  Converting text to integer array...\n");
    // Convert text to int array - exactly like Java code
    for (int i = 0; i < n; i++) {
        g_text[i] = (unsigned char)input_text[i];
    }

    printf("  Starting suffix array construction algorithm...\n");

    // Allocate counting array with max of alphabet size or n
    int max_size = (ALPHABET_SZ > n) ? ALPHABET_SZ : n;
    g_c = (int*)calloc(max_size, sizeof(int));
    if (!g_c) {
        printf("Error: Failed to allocate counting array\n");
        CleanUpGlobals();
        return NULL;
    }

    printf("  Initial counting sort...\n");
    // Initial counting sort - exactly like Java
    for (int i = 0; i < n; i++) {
        g_rank[i] = g_text[i];
        g_c[g_rank[i]]++;
    }

    for (int i = 1; i < ALPHABET_SZ; i++) {
        g_c[i] += g_c[i - 1];
    }

    for (int i = n - 1; i >= 0; i--) {
        g_sa[--g_c[g_text[i]]] = i;
    }

    printf("  Starting doubling iterations...\n");
    // Doubling approach - exactly like Java
    for (int p = 1; p < n; p <<= 1) {
        printf("    Iteration with p=%d...\n", p);

        int r = 0;

        // First part: suffixes that don't have second key
        for (int i = n - p; i < n; i++) {
            g_sa2[r++] = i;
        }

        // Second part: suffixes that have second key
        for (int i = 0; i < n; i++) {
            if (g_sa[i] >= p) {
                g_sa2[r++] = g_sa[i] - p;
            }
        }

        // Clear counting array up to current alphabet size
        for (int i = 0; i < ALPHABET_SZ; i++) {
            g_c[i] = 0;
        }

        // Count ranks
        for (int i = 0; i < n; i++) {
            g_c[g_rank[i]]++;
        }

        // Prefix sum
        for (int i = 1; i < ALPHABET_SZ; i++) {
            g_c[i] += g_c[i - 1];
        }

        // Radix sort by first key
        for (int i = n - 1; i >= 0; i--) {
            g_sa[--g_c[g_rank[g_sa2[i]]]] = g_sa2[i];
        }

        // Update ranks
        g_sa2[g_sa[0]] = r = 0;
        for (int i = 1; i < n; i++) {
            // Check if current and previous have same rank
            int same_rank = (g_rank[g_sa[i - 1]] == g_rank[g_sa[i]]);
            int both_have_second = (g_sa[i - 1] + p < n && g_sa[i] + p < n);
            int same_second = both_have_second && (g_rank[g_sa[i - 1] + p] == g_rank[g_sa[i] + p]);

            if (!(same_rank && both_have_second && same_second)) {
                r++;
            }
            g_sa2[g_sa[i]] = r;
        }

        // Swap rank arrays
        int* tmp = g_rank;
        g_rank = g_sa2;
        g_sa2 = tmp;

        // Update alphabet size
        if (r == n - 1) {
            printf("    Converged early\n");
            break;
        }
        ALPHABET_SZ = r + 1;

        // Reallocate counting array if needed
        if (ALPHABET_SZ > max_size) {
            max_size = ALPHABET_SZ;
            free(g_c);
            g_c = (int*)calloc(max_size, sizeof(int));
            if (!g_c) {
                printf("Error: Failed to reallocate counting array\n");
                CleanUpGlobals();
                return NULL;
            }
        }
    }

    printf("  Suffix array construction completed\n");

    // Copy result
    int* result = (int*)malloc(n * sizeof(int));
    if (!result) {
        printf("Error: Failed to allocate result array\n");
        CleanUpGlobals();
        return NULL;
    }

    memcpy(result, g_sa, n * sizeof(int));

    // Clean up global arrays (except result)
    free(g_text); g_text = NULL;
    free(g_sa2); g_sa2 = NULL;
    free(g_rank); g_rank = NULL;
    free(g_c); g_c = NULL;
    free(g_sa); g_sa = NULL;  // We copied this to result

    return result;
}

// Kasai's algorithm for LCP - same as before but with better error handling
int* ComputeLCPArray(char* text, int* suffix_array, int n) {
    printf("  Allocating memory for LCP computation...\n");

    g_lcp = (int*)malloc(n * sizeof(int));
    int* rank = (int*)malloc(n * sizeof(int));

    if (!g_lcp || !rank) {
        printf("Error: Failed to allocate memory for LCP computation\n");
        if (g_lcp) free(g_lcp);
        if (rank) free(rank);
        return NULL;
    }

    printf("  Computing rank array...\n");
    // Compute rank array (inverse of suffix array)
    for (int i = 0; i < n; i++) {
        rank[suffix_array[i]] = i;
    }

    printf("  Computing LCP values using Kasai's algorithm...\n");
    int k = 0;
    g_lcp[0] = 0;

    for (int i = 0; i < n; i++) {
        if (rank[i] == 0) {
            k = 0;
            continue;
        }

        int j = suffix_array[rank[i] - 1];

        while (i + k < n && j + k < n && text[i + k] == text[j + k]) {
            k++;
        }

        g_lcp[rank[i] - 1] = k;

        if (k > 0) {
            k--;
        }

        // Progress indicator for large files
        if (i % (n / 20) == 0) {
            printf("    Progress: %d%%\n", (int)(100.0 * i / n));
        }
    }

    printf("  LCP computation completed\n");

    free(rank);

    // Copy result and cleanup
    int* result = (int*)malloc(n * sizeof(int));
    if (!result) {
        printf("Error: Failed to allocate LCP result array\n");
        free(g_lcp);
        g_lcp = NULL;
        return NULL;
    }

    memcpy(result, g_lcp, n * sizeof(int));
    free(g_lcp);
    g_lcp = NULL;

    return result;
}

char* ReadFile(const char* filename, long* file_size) {
    printf("Opening file: %s\n", filename);
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("File size: %ld bytes (%.2f MB)\n", *file_size, *file_size / (1024.0 * 1024.0));

    char* buffer = (char*)malloc(*file_size + 1);
    if (!buffer) {
        printf("Error: Cannot allocate memory for file (%ld bytes)\n", *file_size);
        fclose(file);
        return NULL;
    }

    printf("Reading file into memory...\n");
    size_t bytes_read = fread(buffer, 1, *file_size, file);
    if (bytes_read != *file_size) {
        printf("Error: Could not read entire file (read %zu, expected %ld)\n", bytes_read, *file_size);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[*file_size] = '\0';
    fclose(file);
    printf("File read successfully\n");
    return buffer;
}

int WriteLCPToBinary(const char* filename, int* lcp_array, int n, endian_t endianness) {
    printf("Creating output file: %s\n", filename);
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Cannot create output file '%s'\n", filename);
        return 1;
    }

    printf("Writing LCP array to binary file...\n");
    for (int i = 0; i < n; i++) {
        unsigned int value = (unsigned int)lcp_array[i];
        unsigned char bytes[4];

        if (endianness == ENDIAN_BIG) {
            bytes[0] = (value >> 24) & 0xFF;
            bytes[1] = (value >> 16) & 0xFF;
            bytes[2] = (value >> 8) & 0xFF;
            bytes[3] = value & 0xFF;
        } else {
            bytes[3] = (value >> 24) & 0xFF;
            bytes[2] = (value >> 16) & 0xFF;
            bytes[1] = (value >> 8) & 0xFF;
            bytes[0] = value & 0xFF;
        }

        if (fwrite(bytes, 4, 1, file) != 1) {
            printf("Error: Failed to write LCP value at index %d\n", i);
            fclose(file);
            return 1;
        }

        // Progress indicator
        if (i % (n / 20) == 0) {
            printf("  Write progress: %d%%\n", (int)(100.0 * i / n));
        }
    }

    fclose(file);
    printf("Output file written successfully\n");
    return 0;
}

int main(int argc, char* argv[]) {
    endian_t endianness = ENDIAN_LITTLE;
    int arg_index = 1;
    const char* input_file = NULL;
    const char* output_file = NULL;

    // Parse arguments
    while (arg_index < argc) {
        if (strcmp(argv[arg_index], "--little-endian") == 0 || strcmp(argv[arg_index], "-l") == 0) {
            endianness = ENDIAN_LITTLE;
            arg_index++;
        } else if (strcmp(argv[arg_index], "--big-endian") == 0 || strcmp(argv[arg_index], "-b") == 0) {
            endianness = ENDIAN_BIG;
            arg_index++;
        } else if (strcmp(argv[arg_index], "--help") == 0 || strcmp(argv[arg_index], "-h") == 0) {
            printf("LCP Array Constructor\n");
            printf("Usage: %s [options] <input_file> <output_file>\n", argv[0]);
            printf("Options:\n");
            printf("  -l, --little-endian    Output in little-endian format (default)\n");
            printf("  -b, --big-endian       Output in big-endian format\n");
            printf("  -h, --help             Show this help\n");
            return 0;
        } else {
            if (input_file == NULL) {
                input_file = argv[arg_index];
            } else if (output_file == NULL) {
                output_file = argv[arg_index];
            } else {
                printf("Error: Too many arguments\n");
                return 1;
            }
            arg_index++;
        }
    }

    if (input_file == NULL || output_file == NULL) {
        printf("LCP Array Constructor \n");
        printf("Usage: %s [options] <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    printf("LCP Array Constructor\n");
    printf("=================================\n");
    printf("Input file: %s\n", input_file);
    printf("Output file: %s\n", output_file);
    printf("Endianness: %s\n", endianness == ENDIAN_BIG ? "big-endian" : "little-endian");
    printf("\n");

    // Read input file
    long file_size;
    char* text = ReadFile(input_file, &file_size);
    if (!text) {
        return 1;
    }

    int n = (int)file_size;

    printf("\nStarting suffix array construction...\n");
    clock_t start = clock();

    int* suffix_array = ConstructSuffixArray(text, n);
    if (!suffix_array) {
        printf("Error: Suffix array construction failed\n");
        free(text);
        CleanUpGlobals();
        return 1;
    }

    clock_t sa_time = clock();
    printf("Suffix array construction time: %.2f seconds\n", (double)(sa_time - start) / CLOCKS_PER_SEC);

    printf("\nStarting LCP array computation...\n");
    int* lcp_array = ComputeLCPArray(text, suffix_array, n);
    if (!lcp_array) {
        printf("Error: LCP array computation failed\n");
        free(text);
        free(suffix_array);
        CleanUpGlobals();
        return 1;
    }

    clock_t lcp_time = clock();
    printf("LCP computation time: %.2f seconds\n", (double)(lcp_time - sa_time) / CLOCKS_PER_SEC);

    printf("\nWriting output...\n");
    if (WriteLCPToBinary(output_file, lcp_array, n, endianness) != 0) {
        printf("Error: Failed to write output file\n");
        free(text);
        free(suffix_array);
        free(lcp_array);
        CleanUpGlobals();
        return 1;
    }

    printf("\nResults:\n");
    printf("========\n");
    printf("Total processing time: %.2f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    printf("Output file size: %d bytes (%.2f MB)\n", n * 4, (n * 4) / (1024.0 * 1024.0));
    printf("Success! LCP array written to: %s\n", output_file);

    free(text);
    free(suffix_array);
    free(lcp_array);
    CleanUpGlobals();

    return 0;
}
