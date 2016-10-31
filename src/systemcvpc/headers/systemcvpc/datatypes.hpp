/*
 * ----------------------------------------------------------------------------
 *     Copyright (C) 2004-2011 by University of Erlangen-Nuremberg,
 *     Department of Computer Science,
 *     Chair of Hardware-Software-Co-Design, Germany.
 *     All rights reserved.
 * 
 * Project title: SystemC-VPC
 * Comment:
 * ----------------------------------------------------------------------------
 */

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include <stddef.h>

namespace SystemC_VPC
{

typedef size_t ComponentId;
typedef size_t ProcessId;
typedef size_t FunctionId;

// set for debugging output
//#define VPC_DEBUG true;


/******************************************************************************
 *
 */
template<typename ID_TYPE>
class SequentiallyIdedObject
{
public:
  typedef ID_TYPE IdType;

  SequentiallyIdedObject() :
    id_(getNextId())
  {
  }

protected:
  IdType getSequentialId() const
  {
    return id_;
  }

private:
  const IdType id_;

  static IdType getNextId()
  {
    static IdType currentId = 0;
    return currentId++;
  }
};

} // namespace SystemC_VPC
#endif // DATATYPES_H_
