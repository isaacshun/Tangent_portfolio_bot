#include <stdio.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <numeric>

double var(std::vector<double> vec){
  double mean = std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
  double sq_sum = std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.0);
  double var = sq_sum / vec.size() - mean * mean;
  return var;
}

double sd(std::vector<double> vec){
  return std::sqrt(var(vec));
}

double dRand(double min, double max){
  double a = (double)rand() / RAND_MAX;
  return min + a*(max-min);
}