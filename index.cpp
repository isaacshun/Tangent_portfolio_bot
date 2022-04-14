#define GLFW_STATICLIB

#include "Dependencies/GL/glew.h"
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <GL/GL.h>
#include <GL/GLU.h>
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/nlopt/nlopt.hpp"
#include "yfapi.hpp"



int main(){
  
  yfapi::YahooFinanceAPI api;
  api.set_interval(DAILY);

  // Variables
  std::string start = "2021-01-31";
  std::string end = "2021-12-31";
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
    stock = api.get_ticker_data(tickers[i], start, end, "Adj Close", keep_file);
    stock_returns = api.returns(stock);
    stocks.push_back(stock_returns);
  } 
  

  // Getting the risk-free rate
  double rfr = api.risk_free_rate();
  
  // Generating the efficient frontier through simulation
  int portfolio_num = 100;
  std::vector<std::vector<double>> weights;
  std::vector<double> weight;
  std::vector<double> expected_return;
  double exp_r;
  std::vector<double> expected_volatility;
  double exp_v;
  std::vector<double> sharpe;

  double mean = api.portfolio_mean_ret(stocks);
  double cov = api.portfolio_cov(stocks);

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
      exp_r += (weight[j] * mean);
    }
    exp_v = api.exp_volatility(cov, weight);
    expected_return.push_back(exp_r);
    expected_volatility.push_back(exp_v);
    sharpe.push_back(exp_r/exp_v);
  }

  nlopt::opt(NLOPT_GD_STOGO, stocks.size());
  ~nlopt::opt();
  /*
  //Drawing the scatterplot
  GLFWwindow* window;

  if(!glfwInit()){
    return -1;
  }

  

  window = glfwCreateWindow(640, 480, "Efficient frontier", NULL, NULL);
  if(!window){
    glfwTerminate();
    return -1;
  }

  if(glewInit() != GLEW_OK){
    std::cout << "Error!" << std::endl;
  }

  glfwMakeContextCurrent(window);

  while(!glfwWindowShouldClose(window)){
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_2D, 0, portfolio_num);


    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();
  */
  return 0;
}