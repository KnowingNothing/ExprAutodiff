/****************************************************/
/* File: genIR.h                                    */
/****************************************************/

#ifndef __GENIR_H__
#define __GENIR_H__

#include <string>
#include <iostream>

#include "IR.h"
#include "IRMutator.h"
#include "IRVisitor.h"
#include "IRPrinter.h"
#include "type.h"

using namespace Boost::Internal;

Stmt outinit(char* name);

#endif