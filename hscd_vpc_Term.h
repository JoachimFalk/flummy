/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: SystemC-VPC
 * Comment:
 * -----------------------------------------------------------------------------
 * hscd_vpc_Director.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#ifndef HSCD_VPC_TERM_H
#define HSCD_VPC_TERM_H

#include <string>
#include <map>
#include <set>
#include "hscd_vpc_datatypes.h"
#include "hscd_vpc_Director.h"
#include "hscd_vpc_PCBPool.h"

using std::string;
using std::map;
using std::set;
using std::vector;

namespace SystemC_VPC{
#define START_STR "starting"
#define END_STR "ending"
  enum activation_rule {start, end};
#define LESS_STR "less"
#define GREATER_STR "greater"
  enum operator_type {less,greater};
  /**
   * \brief Term represents a boolean term.
   */
  class Term{
    public:
      virtual bool isSatisfied(set<string> excludes)=0;
      virtual bool stillSatisfiable(set<string> excludes){
        return (satisfiableCounter>0);
      }
    protected:
      /// counts how often this term is satisfiable
      int satisfiableCounter;

      virtual ~Term() {}
  };

  /**
   * \brief A Literal represents the boolean value of an expression.
   *
   * An expression contains  the ProcessControlBlock of a task, the activation count for this task and
   * an activation rule. Depending to the rule the expression gets true at start (aktivation)
   * or at end of task execution.  
   */
  class Literal : public Term{
    public:
      Literal(ProcessControlBlock &pcb, operator_type op, int activation,activation_rule rule):
        task(pcb),operation(op),activation(activation),rule(rule)
      {
        satisfiableCounter=1;
      }
      virtual ~Literal(){}
    private:
      ///task
      ProcessControlBlock task;

      /// opertion in expression
      operator_type operation;

      ///activation count
      int activation;

      /// activation rule
      activation_rule rule;
  };

  /**
   *
   */
  class ComplexTerm : public Term{
    public:
      virtual bool isSatisfied(set<string> excludes)=0;
      virtual bool stillSatisfiable()=0;
      void addTerm(Term *t){
        terms.push_back(t);
      }
      virtual ~ComplexTerm(){}
    protected:
      std::vector<Term*> terms;
  };


  /**
   *
   */
  class Disjunction: public ComplexTerm{
    public:
      Disjunction(){
        satisfiableCounter=1;
      }
      bool isSatisfied(set<string> excludes){
        satisfiableCounter--;
        std::vector<Term*>::const_iterator iter=terms.begin();
        for(;iter!=terms.end();iter++){
          if((*iter)->isSatisfied(excludes))return true;
        }
        return false;
      }
  };

  /**
   *
   */
  class AnyTerm : public Term{
    public:
      AnyTerm(const char* srule);
      bool isSatisfied(set<string> excludes);
    protected:
      activation_rule rule;

  };
  /**
   *
   */
  class Constraint{
    private:
      set<string> excludes;
      Term *term;
      char name[VPC_MAX_STRING_LENGTH];
      int count;
      int divider;
      double activationTime;

    public:
      Constraint(const char *name,  const char *count,  const char *divider);
      void addExclude(const char *name);
      bool isSatisfied();
      void addAnyTerm(const char *state);
      void getReport();
      char* getName();
      double getSatisfiedTime();
  };

}
#endif // HSCD_VPC_TERM_H
