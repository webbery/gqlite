#pragma once

class GPersistence {
public:
  /**
   * @brief 
   */
  virtual int Init(const char* filename) = 0;
  /**
   * @brief write data to disk
   */
  virtual int WriteDisk() = 0;

protected:
  
};