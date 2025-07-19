#ifndef SCUPROJ_PARSER_H
#define SCUPROJ_PARSER_H

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

#include <cassert>
#include <cstring>

namespace parser {

std::unordered_map<std::string, void *>
parse(const std::vector<unsigned char> &raw);

std::string
vec_to_dbstr(std::vector<unsigned char> &vec);

std::vector<unsigned char> *
dbstr_to_vec(std::string &str);


} // namespace parser

#endif // SCUPROJ_PARSER_H
