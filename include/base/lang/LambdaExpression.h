#pragma once

struct GListNode;
class GLambdaExpression {
public:
  GLambdaExpression(GListNode* args, GListNode* block);
  ~GLambdaExpression();

private:
  GListNode* _args;
  GListNode* _states;
};