#pragma once

struct GListNode;
class GLambdaExpression {
public:
  GLambdaExpression(GListNode* args, GListNode* block);
  ~GLambdaExpression();

  GListNode* block() const { return _states; }
private:
  GListNode* _args;
  GListNode* _states;
};