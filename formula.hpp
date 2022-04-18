#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <numeric>


/*
Purpose: Returns the covariance between two vectors
Time complexity: O(n), where n is the amount of items in the first vector
Requirement: the two vectors inputted must be of the same length
*/
double cov(std::vector<double> vec1, std::vector<double> vec2){
  double mean = std::accumulate(vec1.begin(), vec1.end(), 0.0) / vec1.size();
  double mean2 = std::accumulate(vec2.begin(), vec2.end(), 0.0) / vec2.size();
  double sum = 0;
  for(int i = 0; i < vec1.size(); ++i){
    sum += ((vec1[i] - mean) * (vec2[i] - mean2));
  }
  return sum / (vec1.size() - 1);
}

/*
Purpose: Returns the variance of the vector
Time complexity: O(n), where n is the amount of items in the vector
*/
double var(std::vector<double> vec){
  return cov(vec, vec);
}


/*
Purpose: Returns the standard deviation of the vector
Time complexity: O(n), where n is the amount of items in the vector
*/
double sd(std::vector<double> vec){
  return std::sqrt(var(vec));
}

/*
Purpose: A random number generator that generates a random number between min and max
Time complexity: O(1)
*/
double dRand(double min, double max){
  double a = (double)rand() / RAND_MAX;
  return min + a*(max-min);
}

/*
Purpose: Gets the max number in an unsorted vector
Time complexity: O(n), where n is the amount of items in the vector
*/
double vec_max(std::vector<double> vec){
  double largest = 0;
  for(int i = 0; i < vec.size(); ++i){
    if (largest < vec[i]) largest = vec[i];
  }
  return largest;
}

/*
Purpose: Gets the min number in an unsorted vector
Time complexity: O(n), where n is the amount of items in the vector
*/
double vec_min(std::vector<double> vec){
  double min = 1000;
  for(int i = 0; i < vec.size(); ++i){
    if (min > vec[i]) min = vec[i];
  }
  return min;
}

/*
Purpose: Searches for an item within the unsorted vector, and returns the position within the vector
Time complexity: O(n), where n is the amount of items in the vector
*/
int search(std::vector<double> vec, double item){
  for(int i = 0; i < vec.size(); ++i){
    if (item == vec[i]){
      return i;
    }
  }
  return -1;
}