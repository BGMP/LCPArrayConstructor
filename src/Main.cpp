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

////////////////////////////////////////////////////////////////////////////////
// Prototypes:
////////////////////////////////////////////////////////////////////////////////

int CompareSuffixes(const void* a, const void* b);
int CompareSuffixesDirect(const void* a, const void* b);
int* ConstructArraySuffixRadix(char* text, int n);
int* ConstructSuffixArraySimple(char* text, int n);
int* ComputeLCPArray(char* text, int* suffix_array, int n);
char* ReadFile(const char* filename, long* file_size);
int WriteLCPBinary(const char* filename, int* lcp_array, int n);
void PrintLCPStatistics(int* lcp_array, int n);
int main(int argc, char* argv[]);

////////////////////////////////////////////////////////////////////////////////
// Globals:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Procedures:
////////////////////////////////////////////////////////////////////////////////

//
// Comparison function for suffix array construction using radix sort approach
//

int CompareSuffixes(const void* a, const void* b) {
  suffix_t* ia = (suffix_t*)a;
  suffix_t* ib = (suffix_t*)b;

  if (ia->rank[0] == ib->rank[0]) {
    return (ia->rank[1] < ib->rank[1]) ? -1 : (ia->rank[1] > ib->rank[1]) ? 1 : 0;
  }
  return (ia->rank[0] < ib->rank[0]) ? -1 : 1;
}

//
// More efficient suffix comparison for large texts
//

int CompareSuffixesDirect(const void* a, const void* b) {
  int ia = *(const int*)a;
  int ib = *(const int*)b;

  return strcmp(global_text + ia, global_text + ib);
}

//
// Construct suffix array using radix sort approach (O(n log^2 n))
//

int* ConstructArraySuffixRadix(char* text, int n) {
  suffix_t* suffixes = (suffix_t*)malloc(n * sizeof(suffix_t));
  int* suffix_array = (int*)malloc(n * sizeof(int));

  // Initialize suffixes with first character ranks
  for (int i = 0; i < n; i++) {
    suffixes[i].index = i;
    suffixes[i].rank[0] = text[i];
    suffixes[i].rank[1] = (i + 1 < n) ? text[i + 1] : -1;
  }

  // Sort by first 2 characters
  qsort(suffixes, n, sizeof(suffix_t), CompareSuffixes);

  int* ind = (int*)malloc(n * sizeof(int));

  // Continue sorting by 4, 8, 16, ... characters
  for (int k = 4; k < 2 * n; k *= 2) {
    // Assign new ranks based on previous sort
    int rank = 0;
    int prev_rank0 = suffixes[0].rank[0];
    int prev_rank1 = suffixes[0].rank[1];
    suffixes[0].rank[0] = rank;
    ind[suffixes[0].index] = 0;

    for (int i = 1; i < n; i++) {
      if (suffixes[i].rank[0] == prev_rank0 && suffixes[i].rank[1] == prev_rank1) {
        suffixes[i].rank[0] = rank;
      } else {
        prev_rank0 = suffixes[i].rank[0];
        prev_rank1 = suffixes[i].rank[1];
        suffixes[i].rank[0] = ++rank;
      }
      ind[suffixes[i].index] = i;
    }

    // Update next ranks
    for (int i = 0; i < n; i++) {
      int next_index = suffixes[i].index + k / 2;
      suffixes[i].rank[1] = (next_index < n) ? suffixes[ind[next_index]].rank[0] : -1;
    }

    qsort(suffixes, n, sizeof(suffix_t), CompareSuffixes);
  }

  // Extract suffix array
  for (int i = 0; i < n; i++) {
    suffix_array[i] = suffixes[i].index;
  }

  free(suffixes);
  free(ind);
  return suffix_array;
}

//
// Construct suffix array using simple comparison (O(n^2 log n) but simpler)
//

int* ConstructSuffixArraySimple(char* text, int n) {
  int* suffix_array = (int*)malloc(n * sizeof(int));

  // Initialize suffix array with indices
  for (int i = 0; i < n; i++) {
    suffix_array[i] = i;
  }

  // Set global variables for comparison
  global_text = text;
  global_n = n;

  // Sort suffixes
  qsort(suffix_array, n, sizeof(int), CompareSuffixesDirect);

  return suffix_array;
}

//
// Compute LCP array using Kasai's algorithm (O(n))
//

int* ComputeLCPArray(char* text, int* suffix_array, int n) {
  int* lcp = (int*)malloc(n * sizeof(int));
  int* rank = (int*)malloc(n * sizeof(int));

  // Compute rank array (inverse of suffix array)
  for (int i = 0; i < n; i++) {
    rank[suffix_array[i]] = i;
  }

  int k = 0;
  lcp[0] = 0;  // LCP of first suffix with previous (non-existent) is 0

  for (int i = 0; i < n; i++) {
    if (rank[i] == 0) {
      k = 0;
      continue;
    }

    int j = suffix_array[rank[i] - 1];

    // Find LCP of current suffix with previous suffix in sorted order
    while (i + k < n && j + k < n && text[i + k] == text[j + k]) {
      k++;
    }

    lcp[rank[i]] = k;

    if (k > 0) {
      k--;
    }
  }

  free(rank);
  return lcp;
}

//
// Read file into memory
//

char* ReadFile(const char* filename, long* file_size) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    printf("Error: Cannot open file '%s'\n", filename);
    return NULL;
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  *file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Allocate memory (+1 for null terminator)
  char* buffer = (char*)malloc(*file_size + 1);
  if (!buffer) {
    printf("Error: Cannot allocate memory for file (%ld bytes)\n", *file_size);
    fclose(file);
    return NULL;
  }

  // Read file
  size_t bytes_read = fread(buffer, 1, *file_size, file);
  if (bytes_read != *file_size) {
    printf("Error: Could not read entire file\n");
    free(buffer);
    fclose(file);
    return NULL;
  }

  buffer[*file_size] = '\0';  // Null terminate
  fclose(file);

  return buffer;
}

//
// Write LCP array to binary file
//

int WriteLCPBinary(const char* filename, int* lcp_array, int n) {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    printf("Error: Cannot create output file '%s'\n", filename);
    return 1;
  }

  // Write as unsigned integers (compatible with DACs)
  for (int i = 0; i < n; i++) {
    unsigned int value = (unsigned int)lcp_array[i];
    if (fwrite(&value, sizeof(unsigned int), 1, file) != 1) {
      printf("Error: Failed to write LCP value at index %d\n", i);
      fclose(file);
      return 1;
    }
  }

  fclose(file);
  return 0;
}

//
// Print statistics about LCP array
//

void PrintLCPStatistics(int* lcp_array, int n) {
  long long sum = 0;
  int max_val = 0;
  int min_val = lcp_array[0];

  // Count frequency of values for median
  int* freq = (int*)calloc(n, sizeof(int));

  for (int i = 0; i < n; i++) {
    sum += lcp_array[i];
    if (lcp_array[i] > max_val) max_val = lcp_array[i];
    if (lcp_array[i] < min_val) min_val = lcp_array[i];

    if (lcp_array[i] < n) {
      freq[lcp_array[i]]++;
    }
  }

  // Find median
  int median = 0;
  int count = 0;
  for (int i = 0; i < n && count < n/2; i++) {
    count += freq[i];
    if (count >= n/2) {
      median = i;
      break;
    }
  }

  // Find most frequent value
  int most_freq_val = 0;
  int most_freq_count = 0;
  for (int i = 0; i < n; i++) {
    if (freq[i] > most_freq_count) {
      most_freq_count = freq[i];
      most_freq_val = i;
    }
  }

  printf("\nLCP Array Statistics:\n");
  printf("====================\n");
  printf("Number of elements: %d\n", n);
  printf("Maximum value: %d\n", max_val);
  printf("Average value: %.2f\n", (double)sum / n);
  printf("Median value: %d\n", median);
  printf("Most frequent value: %d (%.2f%%)\n",
         most_freq_val, (100.0 * most_freq_count) / n);
  printf("Binary file size: %d bytes\n", n * (int)sizeof(unsigned int));

  free(freq);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("LCP Array Constructor\n");
    printf("Usage: %s <input_text_file> <output_binary_file>\n", argv[0]);
    printf("\nThis program constructs the LCP array from a text file\n");
    printf("and outputs it as a binary file compatible with DACs.\n");
    printf("\nExample: %s dna.100MB dna_lcp.bin\n", argv[0]);
    return 1;
  }

  const char* input_file = argv[1];
  const char* output_file = argv[2];

  printf("LCP Array Constructor\n");
  printf("Input file: %s\n", input_file);
  printf("Output file: %s\n", output_file);

  // Read input file
  printf("\nReading input file...\n");
  long file_size;
  char* text = ReadFile(input_file, &file_size);
  if (!text) {
    return 1;
  }

  int n = (int)file_size;
  printf("File size: %ld bytes (%d characters)\n", file_size, n);

  // Choose algorithm based on file size
  printf("\nConstructing suffix array...\n");
  clock_t start_time = clock();

  int* suffix_array;
  if (n > 1000000) {  // 1MB threshold
    printf("Using radix sort algorithm (faster for large files)...\n");
    suffix_array = ConstructArraySuffixRadix(text, n);
  } else {
    printf("Using simple comparison algorithm...\n");
    suffix_array = ConstructSuffixArraySimple(text, n);
  }

  clock_t sa_time = clock();
  printf("Suffix array construction time: %.2f seconds\n",
         (double)(sa_time - start_time) / CLOCKS_PER_SEC);

  // Compute LCP array
  printf("\nComputing LCP array using Kasai's algorithm...\n");
  int* lcp_array = ComputeLCPArray(text, suffix_array, n);

  clock_t lcp_time = clock();
  printf("LCP computation time: %.2f seconds\n",
         (double)(lcp_time - sa_time) / CLOCKS_PER_SEC);

  // Write binary output
  printf("\nWriting LCP array to binary file...\n");
  if (WriteLCPBinary(output_file, lcp_array, n) != 0) {
    printf("Failed to write output file\n");
    free(text);
    free(suffix_array);
    free(lcp_array);
    return 1;
  }

  PrintLCPStatistics(lcp_array, n);

  printf("\nTotal processing time: %.2f seconds\n",
         (double)(clock() - start_time) / CLOCKS_PER_SEC);
  printf("LCP array successfully written to '%s'\n", output_file);

  // Cleanup
  free(text);
  free(suffix_array);
  free(lcp_array);

  return 0;
}
