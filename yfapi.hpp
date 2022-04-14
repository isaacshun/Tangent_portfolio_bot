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
      std::vector<double> get_ticker_data(std::string ticker, std::string start_date, std::string end_date, std::string column, bool keep_file);
      std::string download_ticker_data(std::string ticker, std::string start_date, std::string end_date);
      double risk_free_rate();
      std::vector<double> returns(std::vector<double> vec);
      double portfolio_mean_ret(std::vector<std::vector<double>> vec);
      double portfolio_cov(std::vector<std::vector<double>> vec);
      double exp_volatility(double Sigma, std::vector<double> weight);

    private:
      std::string _base_url;
      Interval _interval;
      std::string build_url(std::string ticker, std::string start_date, std::string end_date);
      bool string_replace(std::string& str, const std::string from, const std::string to);
      std::string timestamp_from_string(std::string date);
      void download_file(std::string url, std::string filename);
  };

  // Constructor 
  YahooFinanceAPI::YahooFinanceAPI(){
    this->_base_url = "https://query1.finance.yahoo.com/v7/finance/download/{ticker}?period1={start_time}&period2={end_time}&interval={interval}&events=history";
    this->_interval = DAILY;
  }

  //Writes the timestamp for the url
  std::string YahooFinanceAPI::timestamp_from_string(std::string date){
    struct std::tm time = {0,0,0,0,0,0,0,0,0};
    std::istringstream ss(date);
    ss >> std::get_time(&time, "%Y-%m-%d");
    if(ss.fail()){
      std::cerr << "ERROR: Cannot parse date string (" << date << "); required format %Y-%m-%d" << std::endl;
      exit(1);
    }
    time.tm_hour = 0;
    time.tm_min = 0;
    time.tm_sec = 0;
    std::time_t epoch = std::mktime(&time);

    return std::to_string(epoch);
  }

  bool YahooFinanceAPI::string_replace(std::string& str, const std::string from, const std::string to){
    size_t start = str.find(from);
    if(start == std::string::npos){
      return false;
    }
    str.replace(start, from.length(), to);
    return true;
  }

  //Builds the URL for grabbing data
  std::string YahooFinanceAPI::build_url(std::string ticker, std::string start, std::string end){
    std::string url = this->_base_url;
    string_replace(url, "{ticker}", ticker);
    string_replace(url, "{start_time}", timestamp_from_string(start));
    string_replace(url, "{end_time}", timestamp_from_string(end));
    string_replace(url, "{interval}", get_api_interval_value(this->_interval));
    return url;
  }

  // Sets the interval
  void YahooFinanceAPI::set_interval(Interval interval){
    this->_interval = interval;
  }

  // Downloads the CSV
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

  //Writes a temporary file and stores data
  std::vector<double> YahooFinanceAPI::get_ticker_data(std::string ticker, std::string start, std::string end, std::string column, bool keep_file){
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
  
  // Returns the latest 10 Year Treasury Yield
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
    return arr[0];
  }
 
  // Writes the CSV downloaded into a csv file
  std::string YahooFinanceAPI::download_ticker_data(std::string ticker, std::string start, std::string end){
    std::string url = build_url(ticker, start, end);
    std::string output_file = ticker + ".csv";
    download_file(url, output_file);

    return output_file;
  }

  // Gets the log returns of the vector
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

  double YahooFinanceAPI::portfolio_mean_ret(std::vector<std::vector<double>> vec){
    double mean;
    for (int i = 0; i < vec.size(); ++i){
      mean += (std::accumulate(vec[i].begin(), vec[i].end(), 0.0) / vec[i].size());
    }
    mean /= vec.size();
    return mean;
  }

  double YahooFinanceAPI::portfolio_cov(std::vector<std::vector<double>> vec){
    if (vec.size() == 1){
      return var(vec[0]);
    } else {
      std::vector<double> sum_vec;
      double sum, mean;
      double cov = 1;
      for(int i = 0; i < vec.size(); ++i){
        mean = (std::accumulate(vec[i].begin(), vec[i].end(), 0.0) / vec[i].size());
        for(int j = 0; j < vec[i].size(); ++j){
          sum += (vec[i][j] - mean);
        }
        sum_vec.push_back(sum);
      }
      for(int i = 0; i < sum_vec.size(); ++i){
        cov *= sum_vec[i];
      }
      cov /= (sum_vec.size() - 1);
      return cov;
    }
  }

  double YahooFinanceAPI::exp_volatility(double Sigma, std::vector<double> weight){
    std::vector<double> temp;
    double sum;
    for(int i = 0; i < weight.size(); ++i){
      temp.push_back(Sigma * weight[i]);
    }
    for(int j = 0; j < weight.size(); ++j){
      sum += (weight[j] * temp[j]);
    }
    return sqrt(sum);
  }
}