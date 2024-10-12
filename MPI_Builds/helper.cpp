#include <cstdlib>
#include <ctime>
#include <algorithm> // For std::swap

// Sorted
int* sortedArray(int n) {
    int* a = new int[n]; 
    for (int i = 0; i < n; i++) {
        a[i] = i; 
    }
    return a; 
}

// Random
int* randomArray(int n) {
    int* a = new int[n];
    for (int i = 0; i < n; i++) {
        a[i] = rand() % n;
    }
    return a;
}

// Reverse sorted
int* reverseArray(int n) {
    int* a = new int[n];
    for (int i = 0; i < n; i++) {
        a[i] = n - i - 1;
    }
    return a;
}

int* perturbedArray(int n) {
    int* a = new int[n];
    for (int i = 0; i < n; i++) {
        a[i] = i; // Fill the array with sorted values
    }
    for (int i = 0; i < n / 100; i++) {
        int x = rand() % n; // Random index x
        int y = rand() % n; // Random index y
        // Swap elements at indices x and y
        int temp = a[x];
        a[x] = a[y];
        a[y] = temp;
    }
    return a;
}


void deleteArray(int* a) {
    delete[] a;
}