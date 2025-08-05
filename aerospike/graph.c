#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
// No other libraries needed/allowed.

static const int adj[109][12] = {
    {1, 2, 3, 4, 5, 6, -1}, // 0
    {2, 6, 7, 8, 9, 10, 11, 73, 109, -1}, // 1
    {3, 12, 57, 102, 103, 104, 105, 106, 107, 108, 109, -1}, // 2
    {4, 12, 13, -1}, // 3
    {5, 13, 14, -1}, // 4
    {6, 14, 15, -1}, // 5
    {7, 15, 16, -1}, // 6
    {8, 16, 17, -1}, // 7
    {9, 17, 18, -1}, // 8
    {10, 18, 19, -1}, // 9
    {11, 19, 20, -1}, // 10
    {20, 73, -1}, // 11
    {13, 21, 58, 59, -1}, // 12
    {14, 21, 22, -1}, // 13
    {15, 22, 23, -1}, // 14
    {16, 23, 24, -1}, // 15
    {17, 24, 25, -1}, // 16
    {18, 25, 26, -1}, // 17
    {19, 26, 27, -1}, // 18
    {20, 27, 28, -1}, // 19
    {28, 72, 73, -1}, // 20
    {22, 29, 59, 60, -1}, // 21
    {23, 29, 30, -1}, // 22
    {24, 30, 31, -1}, // 23
    {25, 31, 32, -1}, // 24
    {26, 32, 33, -1}, // 25
    {27, 33, 34, -1}, // 26
    {28, 34, 35, -1}, // 27
    {35, 71, 72, -1}, // 28
    {30, 36, 60, 61, -1}, // 29
    {31, 36, 37, -1}, // 30
    {32, 37, 38, -1}, // 31
    {33, 38, 39, -1}, // 32
    {34, 39, 40, -1}, // 33
    {35, 40, 41, -1}, // 34
    {41, 70, 71, -1}, // 35
    {37, 42, 61, 62, -1}, // 36
    {38, 42, 43, -1}, // 37
    {39, 43, 44, -1}, // 38
    {40, 44, 45, -1}, // 39
    {41, 45, 46, -1}, // 40
    {46, 69, 70, -1}, // 41
    {43, 47, 62, 63, -1}, // 42
    {44, 47, 48, -1}, // 43
    {45, 48, 49, -1}, // 44
    {46, 49, 50, -1}, // 45
    {50, 68, 69, -1}, // 46
    {48, 51, 63, 64, -1}, // 47
    {49, 51, 52, -1}, // 48
    {50, 52, 53, -1}, // 49
    {53, 67, 68, -1}, // 50
    {52, 54, 64, 65, -1}, // 51
    {53, 54, 55, -1}, // 52
    {55, 66, 67, -1}, // 53
    {55, 56, 65, -1}, // 54
    {56, 66, -1}, // 55
    {65, 66, -1}, // 56
    {58, 102, -1}, // 57
    {59, 95, 102, -1}, // 58
    {60, 89, 95, -1}, // 59
    {61, 84, 89, -1}, // 60
    {62, 80, 84, -1}, // 61
    {63, 77, 80, -1}, // 62
    {64, 75, 77, -1}, // 63
    {65, 74, 75, -1}, // 64
    {66, 74, -1}, // 65
    {67, 74, -1}, // 66
    {68, 74, 76, -1}, // 67
    {69, 76, 79, -1}, // 68
    {70, 79, 83, -1}, // 69
    {71, 83, 88, -1}, // 70
    {72, 88, 94, -1}, // 71
    {73, 94, 101, -1}, // 72
    {101, 109, -1}, // 73
    {75, 76, -1}, // 74
    {76, 77, 78, -1}, // 75
    {78, 79, -1}, // 76
    {78, 80, 81, -1}, // 77
    {79, 81, 82, -1}, // 78
    {82, 83, -1}, // 79
    {81, 84, 85, -1}, // 80
    {82, 85, 86, -1}, // 81
    {83, 86, 87, -1}, // 82
    {87, 88, -1}, // 83
    {85, 89, 90, -1}, // 84
    {86, 90, 91, -1}, // 85
    {87, 91, 92, -1}, // 86
    {88, 92, 93, -1}, // 87
    {93, 94, -1}, // 88
    {90, 95, 96, -1}, // 89
    {91, 96, 97, -1}, // 90
    {92, 97, 98, -1}, // 91
    {93, 98, 99, -1}, // 92
    {94, 99, 100, -1}, // 93
    {100, 101, -1}, // 94
    {96, 102, 103, -1}, // 95
    {97, 103, 104, -1}, // 96
    {98, 104, 105, -1}, // 97
    {99, 105, 106, -1}, // 98
    {100, 106, 107, -1}, // 99
    {101, 107, 108, -1}, // 100
    {108, 109, -1}, // 101
    {103, -1}, // 102
    {104, -1}, // 103
    {105, -1}, // 104
    {106, -1}, // 105
    {107, -1}, // 106
    {108, -1}, // 107
    {109, -1}, // 108
};

static int graph[110][24] = {0};
static int counts[110] = {0};
static int colors[110] = {0};

static void
fill_graph()
{
  for (int node = 0; node < 109; node++) {
    for (int i = 0; i < 12; i++) {
      if (adj[node][i] == -1) {
        break; 
      }
      int neighbor = adj[node][i];
      graph[node][counts[node]] = neighbor;
      counts[node]++;
      graph[neighbor][counts[neighbor]] = node;
      counts[neighbor]++;
    }
  }
}
static bool
is_color_ok(int c, int node) {
   for (int i=0; i<counts[node]; ++i) {
       int neighbor = graph[node][i];
       if (colors[neighbor] == c) {
         return false;
       }
   }
   return true;
}
static int
fill_colors_helper(int node) {
  for (int c = 1; c <= 4; ++c) {
    if (is_color_ok(c, node)) {
      colors[node] = c;
      bool success = true;
      for (int i = 0; i < counts[node]; ++i) {
        int neighbor = graph[node][i];
        if (colors[neighbor] == 0) {
          if (fill_colors_helper(neighbor) == -1) {
            success = false;
            break;
          }
        }
      }
      if (success) {
        return 0;
      }
      colors[node] = 0; //retry another color
    }
  }
  return -1;  // No valid color
}


// Structs and functions here.
static void
fill_colors()
{
  // TODO - fill colors
  //need a loop here to ensure that the code works for disconnected graphs as well.
  for (int i = 0; i < 110; ++i) {
    if (colors[i] == 0) {
      if (fill_colors_helper(i) == -1) {
          printf("Failed to color graph with 4 colors\n");
          return;
      }
    }
  }
}

static void
answer()
{
    // Print color nodes.
    for (int i = 0; i < 110; i++) {
      printf("%d -- %d\n", i, colors[i]);
    }
}

static void
check_graph()
{
  for (int i = 0; i < 110; i++) {
    for (int j = 0; j < counts[i]; j++) {
      int n = graph[i][j];
      if (colors[i] == colors[n]) {
        printf("neighbors %d and %d are color %d", i, n, colors[i]);
        return;
      }
    }
  }
}

int main()
{
    fill_graph();
    fill_colors();
    answer();
    check_graph();
    return 0;
}

