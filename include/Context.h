#pragma once

class GVM;
class GDefaultSchedule;
class GStorageEngine;
class GVirtualNetwork;
class GPlaneNode;

struct GContext {
  GPlaneNode* _root;
  GPlaneNode* _tail;
  /**< true if plan can be run in thread and next plan did not wait it finish. */
  bool _threadable;

  GVM* _gvm;
  GDefaultSchedule* _schedule;
  GStorageEngine* _storage;
  GVirtualNetwork* _graph;
};