#include "ParserEngine.h"
#include "Error.h"

// GParserEngine::GParserEngine()
// {
// }

// GParserEngine::~GParserEngine()
// {
// }

// int GParserEngine::parse(const std::string& gql, AST** ast, char** error)
// {
//     try{
//         grammar::pegtl::memory_input<> inp(gql, "");
//         auto root = grammar::pegtl::parse_tree::parse<
//           grammar::GGrammar, GASTNode, grammar::gql_selector, grammar::GAction > (inp, *this);
//         if (root->is_root()) {
//           *ast = new AST(std::move(root));
//         }
//     } catch(tao::pegtl::parse_error err) {
//         const char* e = err.what();
//         strncpy(*error, e, strlen(e));
//         return ECode_GQL_Grammar_Error;
//     } catch(std::system_error err) {
//         const char* e = err.what();
//         strncpy(*error, e, strlen(e));
//         return ECode_GQL_Grammar_Error;
//     }
//     return 0;
// }

// void GParserEngine::pushAction(const grammar::GActionInfo& action)
// {
//   // printf("push action: %s\n", action.c_str());
//   _actions.push(action);
// }

// bool GParserEngine::popAction(grammar::GActionInfo& action)
// {
//   if (_actions.size() == 0) {
//     printf("pop empty action\n");
//     return false;
//   }
//   action = _actions.top();
//   _actions.pop();
//   return true;
// }
