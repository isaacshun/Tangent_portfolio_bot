#define GLFW_STATICLIB

#include "Dependencies/GL/glew.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <chrono>
#include <gnugraph/GnuGraph.h>
#include "yfapi.hpp"

int main(){
  auto run = std::chrono::high_resolution_clock::now();
  yfapi::YahooFinanceAPI api;
  api.set_interval(DAILY);

  // Variables
  std::time_t end = std::time(0);
  std::time_t start = end - 86400*1826;
  const bool keep_file = false;

  std::vector<std::string> tickers;
  std::string ticker, line;

  std::cout << "Enter the list of stocks in the portfolio"<< std::endl;
  std::getline(std::cin, line);
  std::istringstream stream(line);
  while (stream >> ticker){
    tickers.push_back(ticker);
  }

  // Getting the adjusted close data from the api
  std::vector<double> SPY_Close = api.get_ticker_data("SPY", start, end, "Adj Close", keep_file);
  std::vector<double> SPY_returns = api.returns(SPY_Close);

  std::vector<std::vector<double>> stocks;
  std::vector<double> stock;
  std::vector<double> stock_returns;
  
  for (int i = 0; i < tickers.size(); ++i){
    
    stock.clear();
    stock_returns.clear();
    stock = api.get_ticker_data(tickers[i], start, end, "Close", keep_file);
    stock_returns = api.returns(stock);
    stocks.push_back(stock_returns);
  } 
  

  // Getting the risk-free rate
  double rfr = api.risk_free_rate();
  
  // Generating the efficient frontier through simulation
  int portfolio_num = 25000;
  std::vector<std::vector<double>> weights;
  std::vector<double> weight;
  std::vector<double> expected_return;
  double exp_r;
  std::vector<double> expected_volatility;
  double exp_v;
  std::vector<double> sharpe;
  double sharpe_r;

  std::vector<double> mean = api.portfolio_mean_ret(stocks);

  // Random weight vector generator
  std::srand(time(0));
  for (int i = 0; i < portfolio_num; ++i){
    double sum = 0;
    exp_r = 0;
    exp_v = 0;
    weight.clear();
    for (int j = 0; j < stocks.size(); ++j){
      double random = dRand(0, 100);
      sum += random;
      weight.push_back(random);
    }
    // Make the weights total to 1
    for (int j = 0; j < weight.size(); ++j){
      weight[j] /= sum;
    }
    weights.push_back(weight);

    for (int j = 0; j < weight.size(); ++j){
      exp_r += (weight[j] * mean[j]);
    }
    exp_v = std::sqrt(api.portfolio_var(stocks, weight));
    expected_return.push_back(exp_r);
    expected_volatility.push_back(exp_v);
    //Get annualized sharpe ratio
    sharpe_r = std::sqrt(365)*(exp_r - rfr)/exp_v;
    sharpe.push_back(sharpe_r);
  }
  double max = vec_max(sharpe);
  int index = search(sharpe, max);

  std::cout << "The optimal portfolio allocation is [";
  for (int i = 0; i < weights[0].size()-1; ++i){
    std::cout << weights[index][i] <<", ";
  }
  std::cout << weights[index][weights[0].size()-1] << "] with a sharpe ratio of " << max << std::endl;
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - run);
  std::cout << duration.count() << std::endl;
  
  /*
  //Drawing the scatterplot
  GnuGraph graph("C:/Program Files/gnuplot/bin");
  const std::string output = graph.plot(expected_volatility, expected_return);
  std::cout << output << '\n';
  std::cin.get();
  */
  return 0;
}