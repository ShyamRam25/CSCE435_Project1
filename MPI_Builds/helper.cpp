#include "helper.h"
#include <stdlib.h>

// creating arrays to pass to all different sorting functions

// Sorted, Random, Reverse sorted, 1%perturbed

// Sorted
int* sortedArray(int n){
    int* a;
    a =(int*)malloc(n*sizeof(int));
    for(int i=0; i<n; i++){
        a[i] = i;
    }
    return a;
}


// Random
int* randomArray(int n){
    int* a;
    a = (int*)malloc(n*sizeof(int));
    for(int i=0; i<n; i++){
        a[i] = rand()%n;
    }
    return a;
}

// Reverse sorted
int* reverseArray(int n){
    int* a;
    a = (int*)malloc(n*sizeof(int));
    for(int i=0; i<n; i++){
        a[i] = n-i;
    }
    return a;
}

// 1% perturbed
int* perturbedArray(int n){
    int* a;
    a = (int*)malloc(n*sizeof(int));
    for(int i=0; i<n; i++){
        a[i] = i;
    }
    for(int i=0; i<n/100; i++){
        int x = rand()%n;
        int y = rand()%n;
        int temp = a[x];
        a[x] = a[y];
        a[y] = temp;
    }
    return a;
}