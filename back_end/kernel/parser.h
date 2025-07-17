#ifndef SCUPROJ_PARSER_H
#define SCUPROJ_PARSER_H

#include <unordered_map>
#include <string>
#include <vector>

#include <cassert>
#include <cstring>

namespace parser {

std::unordered_map<std::string, void *>
parse(const std::vector<unsigned char> &raw);

} // namespace parser

#endif // SCUPROJ_PARSER_H
