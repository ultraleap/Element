#pragma once
#include <iterator>
#include <string>
#include <istream>
#include <sstream>
#include <vector>

template<char delimiter>
class WordDelimitedBy : public std::string
{};

template <char c>
std::istream& operator>>(std::istream& is, WordDelimitedBy<c>& output)
{
    std::getline(is, output, c);
    return is;
}

template<char delimiter>
std::vector<std::string> split(std::string input_str)
{
    std::istringstream iss(input_str);
    return { std::istream_iterator<WordDelimitedBy<delimiter>>(iss),
            std::istream_iterator<WordDelimitedBy<delimiter>>() };
}