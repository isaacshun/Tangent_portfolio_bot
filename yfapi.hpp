#define CURL_STATICLIB

#include <string.h>
#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <cmath>
#include <numeric>
#include "Dependencies/curl/include/curl/curl.h"
#include "interval.hpp"
#include "formula.hpp"

namespace yfapi
{
  class YahooFinanceAPI{
    public:
      YahooFinanceAPI();
      void set_interval(Interval interval);
      std::vector<double> get_ticker_data(std::string ticker, std::time_t start_date, std::time_t end_date, std::string column, bool keep_file);
      std::string download_ticker_data(std::string ticker, std::time_t start_date, std::time_t end_date);
      double risk_free_rate();
      std::vector<double> returns(std::vector<double> vec);
      std::vector<double> portfolio_mean_ret(std::vector<std::vector<double>> vec);
      double portfolio_var(std::vector<std::vector<double>> returns, std::vector<double> weight);

    private:
      std::string _base_url;
      Interval _interval;
      std::string build_url(std::string ticker, std::time_t start_date, std::time_t end_date);
      bool string_replace(std::string& str, const std::string from, const std::string to);
      void download_file(std::string url, std::string filename);
  };

  // Constructor 
  YahooFinanceAPI::YahooFinanceAPI(){
    this->_base_url = "https://query1.finance.yahoo.com/v7/finance/download/{ticker}?period1={start_time}&period2={end_time}&interval={interval}&events=history";
    this->_interval = DAILY;
  }

  /*
  Purpose: Replaces a portion of a string
  Time complexity: O(n), where n is the length of str
  */
  bool YahooFinanceAPI::string_replace(std::string& str, const std::string from, const std::string to){
    size_t start = str.find(from);
    if(start == std::string::npos){
      return false;
    }
    str.replace(start, from.length(), to);
    return true;
  }

  /*
  Purpose: A helper function to build the url for downloading data from yahoo finance
  Time complexity: O(n), where n is the length of the string
  */
  std::string YahooFinanceAPI::build_url(std::string ticker, std::time_t start, std::time_t end){
    std::string url = this->_base_url;
    string_replace(url, "{ticker}", ticker);
    string_replace(url, "{start_time}", std::to_string(start));
    string_replace(url, "{end_time}", std::to_string(end));
    string_replace(url, "{interval}", get_api_interval_value(this->_interval));
    return url;
  }

  /*
  Purpose: Sets the interval for the YahooFinanceAPI class
  Time complexity: O(1)
  */
  void YahooFinanceAPI::set_interval(Interval interval){
    this->_interval = interval;
  }

  /*
  Purpose: A helper function for downloading a csv containing financial data and save it as the filename inputed
  Time complexity: O(n), where n is the number of items in the csv
  */
  void YahooFinanceAPI::download_file(std::string url, std::string filename){
    FILE *fp;
    CURLcode res;
    CURL *curl = curl_easy_init();
    
    if (curl){
      fp = fopen(filename.c_str(), "wb");
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
      res = curl_easy_perform(curl);

      /* always cleanup */
      curl_easy_cleanup(curl);
      fclose(fp);
    } else {
      std::cout << "curl init failed" << std::endl;
    }
  }

  /*
  Purpose: Retrieves data from the ticker inputed and returns a vector with the csv's column data. Based on the variable keep_file, it may save the file
  Time Complexity: O(n), where n is the amount of items in the csv
  */
  std::vector<double> YahooFinanceAPI::get_ticker_data(std::string ticker, std::time_t start, std::time_t end, std::string column, bool keep_file){
    // Downloading the file
    std::string url = build_url(ticker, start, end);
    std::string output_file = ticker + ".csv";
    download_file(url, output_file);

    // Open file and store the close column into a vector
    std::fstream f;
    f.open(output_file.c_str(), std::ios::in);
    std::string line, word;
    std::vector<double> arr;
    int i = 0;
    int j = 1;
    int col = 0;

    // Chooses the column based on the column inputted
    
    if (column == "Open"){
      col = 1;
    } else if (column == "High"){
      col = 2;
    } else if (column == "Low"){
      col = 3;
    } else if (column == "Close"){
      col = 4;
    } else if (column == "Adj Close"){
      col = 5;
    } else if (column == "Volume"){
      col = 6;
    } else {
      std::cout << "Not a column" << std::endl;
      return arr;
    }

    while(std::getline(f, line)){
      std::stringstream s(line);

      while(std::getline(s, word, ',')){
        if(i == 7*j+col){
          arr.push_back(std::stod(word));
          ++j;
        }
        ++i;
      }
    }

    f.close();
    if (!keep_file){
      std::remove(output_file.c_str());
    }
    return arr;
  }
  
  /*
  Purpose: Retrieves the latest 10 Year Treasury Rate, and adjusts to monthly
  Time Complexity: O(n), where n is the amount of items in the csv
  */
  double YahooFinanceAPI::risk_free_rate(){
    std::time_t now = time(0);
    std::tm *date = localtime(&now);
    --date->tm_mday;
    std::string url = this->_base_url;
    string_replace(url, "{ticker}", "^TNX");
    string_replace(url, "{start_time}", std::to_string(mktime(date)));
    string_replace(url, "{end_time}", std::to_string(now));
    string_replace(url, "{interval}", get_api_interval_value(this->_interval));
    std::string output_file = "TNX.csv";
    download_file(url, output_file);

    std::fstream f;
    f.open(output_file.c_str(), std::ios::in);
    std::string line, word;
    std::vector<double> arr;

    int i = 0;

    while(std::getline(f, line)){
      std::stringstream s(line);

      while(std::getline(s, word, ',')){
        if(i == 12){
          arr.push_back(std::stod(word));
        }
        ++i;
      }
    }

    f.close();
    std::remove(output_file.c_str());
    return arr[0]/1200;
  }
 
  /*
  Purpose: Writes the CSV downloaded into a csv file
  Time Complexity: O(n), where n is the amount of items in the csv
  */
  std::string YahooFinanceAPI::download_ticker_data(std::string ticker, std::time_t start, std::time_t end){
    std::string url = build_url(ticker, start, end);
    std::string output_file = ticker + ".csv";
    download_file(url, output_file);

    return output_file;
  }

  /*
  Purpose: A helper function that transforms the stock data inputted and returns the log return of the stock into a vector
  Time Complexity: O(n-1) , where n is the number of items in the vector
  */
  std::vector<double> YahooFinanceAPI::returns(std::vector<double> vec){
    double returns = 0;
    std::vector<double> return_vec;
    for(int i = 1; i < vec.size(); i++){
      returns = (vec[i] - vec[i-1])/vec[i];
      // generate log return
      returns = std::log(1 + returns);
      return_vec.push_back(returns);
    }
    return return_vec;
  }

  /*
  Purpose: Returns a vector with the mean of the log return of each security
  Time Complexity: O(n*m), where n is the amount of securities and m is the amount of return data per security
  */
  std::vector<double> YahooFinanceAPI::portfolio_mean_ret(std::vector<std::vector<double>> vec){
    std::vector<double> mean;
    double stock_mean;
    for (int i = 0; i < vec.size(); ++i){
      stock_mean = std::accumulate(vec[i].begin(), vec[i].end(), 0.0);
      stock_mean /= vec[i].size();
      mean.push_back(stock_mean);
    }
    return mean;
  }

  /*
  Purpose: Returns the portfolio variance based of the returns and the weighting put into each security
  Time complexity: O(n*m), where n is the amount of securities and m is the amount of return data per security
  */
  double YahooFinanceAPI::portfolio_var(std::vector<std::vector<double>> returns, std::vector<double> weight){
    double variance;
    if (returns.size() == 1){
      return var(returns[0]);
    } else {
      for(int i = 0; i < returns.size(); ++i){
        for(int j = 0; j < weight.size(); ++j){
          variance += weight[i] * weight[j] * cov(returns[i], returns[j]);
        }
      }
    }
    return variance;
  }
}