/***************************************************************
 **** Other Graph Query Language to gqlite query language
****************************************************************/

#include "converter/ngql.h"
#include <stdio.h>
#ifdef _WIN32
#include "getopt.h"
#else
#include <unistd.h>
#endif
#include <string>
#include "converter/ngql.h"

enum InputFormat {
  IF_NGQL,
};

void ngql_converter() {}

void usage() {
  printf("Usage:\n");
  printf("\tl2lite [-f format:ngql/] [-s source_gql_file] [-o output_file]\n");
}

/**
 * Use: l2glite -fngql -s basketball.ngql -o basketball.lite
*/
int main(int argc, char** argv) {
  int o = 0;
  const char* optstr = "f::s:o:";

  InputFormat format = IF_NGQL;
  IConverter* pConverter = nullptr;
  std::string source, output;
  while ((o = getopt(argc, argv, optstr)) != -1) {
    switch (o)
    {
    case 'f':
      if (strcmp(optarg, "ngql") == 0) {
        pConverter = new NGQLConverter();
        break;
      }
      usage();
      return -1;
    case 's':
      if ((source = optarg).empty()) {
        usage();
        return -1;
      }
      break;
    case 'o':
      if ((output = optarg).empty()) {
        usage();
        return -1;
      }
      break;
    default:
      usage();
      break;
    }
  }
  FILE* fp_reader = NULL;
  FILE* fp_writer = NULL;
  fp_reader = fopen(source.c_str(), "r");
  fp_writer = fopen(output.c_str(), "w+");

#define BUF_SIZE  2048
  char buff[BUF_SIZE] = {0};
  while (fscanf(fp_reader, "%s", buff) != EOF) {
    std::string out;
    if (pConverter->Parse(buff, out) == IConverter::CS && !out.empty()) {
      memset(buff, 0, BUF_SIZE);
      continue;
    }
    memset(buff, 0, BUF_SIZE);
    printf("%s\n", out.c_str());
  }

  fclose(fp_writer);
  fclose(fp_reader);

  delete pConverter;
}