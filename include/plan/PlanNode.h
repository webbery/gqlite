#pragma once

enum class GPlanKind {
  UpsetVertex,
  UpsetEdge,
  DeleteVertex,
  DeleteEdge,
};

class GPlanNode {
public:
//   virtual float cost() const;

protected:
  GPlanKind _kind;
};