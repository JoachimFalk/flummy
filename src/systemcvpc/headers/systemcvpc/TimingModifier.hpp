// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is"
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SYSTEMCVPC_TIMINGMODIFIER_HPP
#define _INCLUDED_SYSTEMCVPC_TIMINGMODIFIER_HPP


#include <boost/random.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/bernoulli_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/cauchy_distribution.hpp>
#include <boost/random/exponential_distribution.hpp>
#include <boost/random/gamma_distribution.hpp>
#include <boost/random/geometric_distribution.hpp>
#include <boost/random/lognormal_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/triangle_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include <systemc>

#include <fstream>
#include <systemc>
#include <vector>
#include <algorithm>

#include <stdio.h>

namespace SystemC_VPC{

  //the base TimingModifier class is used as identity function
  class TimingModifier {
    typedef TimingModifier this_type;
  public:
    typedef boost::shared_ptr<this_type> Ptr;

    //this constructor should result in an identity function
    TimingModifier(){
    }

    virtual sc_core::sc_time rePlay(sc_core::sc_time time){
      return time;
    }

    //fake function for the modifcation
    //Parameter: the time to modify
    //return: the modified time
    virtual sc_core::sc_time modify(sc_core::sc_time time){
      return time;
    }

    virtual double modify(double val){
      return val;
    }

    virtual void reset(){
    }

    virtual ~TimingModifier() {}
  };

  //the TimingModifier for distributions
  class DistributionTimingModifier : public TimingModifier {
  public:
    virtual sc_core::sc_time modify(sc_core::sc_time time){
      this->reRoll();
      this->history.push_back(this->factor);
      this->position++;
      return time*this->factor;
    }
    virtual double modify(double val){
      this->reRoll();
      this->history.push_back(this->factor);
      this->position++;
      return val*this->factor;
    }

    //generates a new random factor used later for modification
    virtual void reRoll(){
      if (this->factor == -1 || !this->isfixed){
        //roll till a suitable factor is found
        while (1){
          double roled = getFactor();
          if (hasBase){
            roled = base->modify(roled);
          }
          if ((roled >= minValue) && ((roled < maxValue) || (maxValue == -1))) {
            this->factor = roled+1;
            break;
          }
        }
      }
    }

    //getfactor is overridden later
    virtual long double getFactor(){
      return 1;
    }

    //drop the modifier history
    virtual void reset(){
      this->position = 0;
      this->history.clear();
    }

    //repeat results from modifier history
    virtual sc_core::sc_time rePlay(sc_core::sc_time time){
      this->position++;
      return time*(this->history[(this->position)-1]);
    }

    //set a base distribution
    virtual void setBase(boost::shared_ptr<TimingModifier> base){
      this->base = base;
      hasBase = true;
    }

  protected:
    //shared pointers were used to ensure that the factor is shared between all copys of this instance
    boost::uniform_real<> random;
    long double factor;
    boost::shared_ptr<boost::mt19937> gen;
    std::vector<long double> history;
    unsigned int position;
    long double minValue;
    long double maxValue;
    bool isfixed;
    boost::shared_ptr<TimingModifier> base;
    bool hasBase;
  };

  //the TimingModifier for Distributions based on example data
  class EmpiricTimingModifier : public DistributionTimingModifier {
  public:

    //approximate a distribution from data
    EmpiricTimingModifier(boost::shared_ptr<boost::mt19937> generator,sc_core::sc_time scale,const char* filename,double minValue,double maxValue,bool isfixed){
      //initialize base values
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;

      //open data file and scale values
      std::ifstream file(filename);
      std::vector<double> data = std::vector<double>();
      while (!file.eof()){
        char tmp[256];
        file.getline(tmp,256);
        data.push_back((sc_core::sc_time(atol(tmp),sc_core::SC_PS)/scale)-1);
      }
      this->leftBorder=std::vector<double>();
      this->height=std::vector<double>();

      //sort data
      sort(data.begin(), data.end());
      this->min = data.front();
      this->max = data.back();
      this->size = data.size();
      this->numStripes = ((int)sqrt(this->size))+1;

      //calculate left borders for every stripe
      unsigned int count = 0;
      while(count<(this->numStripes)){
        leftBorder.push_back(this->min+((this->max-this->min)/this->numStripes)*count);
        count += 1;
      }

      //calculate height for every stripe
      int pos = 0;
      for(unsigned int count = 0; count<data.size(); count++){
         if (data[count]>=this->leftBorder[pos+1] && pos<(this->numStripes-1)){
           this->height.push_back(count);
           pos++;
        }
      }
      this->height.push_back(this->size);
    }

    //get value from approximated distribution
    virtual long double getFactor(){
      //generate random y
      boost::variate_generator<boost::mt19937&,boost::uniform_real<> > gen(*(this->gen.get()),boost::uniform_real<> (0,this->size));
      double pos = gen();

      //calculate stripe
      int count = 0;
      while (pos>this->height[count]){
        count++;
      }

      //calculate x for this stripe
      return this->leftBorder[count-1]+((this->max-this->min)/this->numStripes)+((pos-this->height[count-1])/(this->height[count]-this->height[count-1]))*((this->max-this->min)/this->numStripes);
    }

  private:
    std::vector<double> myvector2;
    boost::uniform_real<> random;
    std::vector<double> leftBorder;
    std::vector<double> height;
    double min;
    double size;
    double max;
    double numStripes;
  };

  //The timingModifier for uniforn distributions
  class UniformRealTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    UniformRealTimingModifier(boost::shared_ptr<boost::mt19937> generator,double min,double max,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::uniform_real<> (min,max);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::uniform_real<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::uniform_real<> random;
  };

  //The timingModifier for bernoulli distributions
  class BernoulliTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    BernoulliTimingModifier(boost::shared_ptr<boost::mt19937> generator,double p,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::bernoulli_distribution<> (p);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::bernoulli_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::bernoulli_distribution<> random;
  };

  //The timingModifier for binomial distributions
  class BinomialTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    BinomialTimingModifier(boost::shared_ptr<boost::mt19937> generator,int t,double p,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::binomial_distribution<> (t,p);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::binomial_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::binomial_distribution<> random;
  };

  //The timingModifier for cauchy distributions
  class CauchyTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    CauchyTimingModifier(boost::shared_ptr<boost::mt19937> generator,double median,double sigma,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::cauchy_distribution<> (median,sigma);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::cauchy_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::cauchy_distribution<> random;
  };

  //The timingModifier for exponential distributions
  class ExponentialTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    ExponentialTimingModifier(boost::shared_ptr<boost::mt19937> generator,double lambda,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::exponential_distribution<> (lambda);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::exponential_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::exponential_distribution<> random;
  };

  //The timingModifier for gamma distributions
  class GammaTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    GammaTimingModifier(boost::shared_ptr<boost::mt19937> generator,double shape,double scale,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::gamma_distribution<> (shape);
      this->gen = generator;
      this->scale = scale;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::gamma_distribution<> > gen(*(this->gen.get()),this->random);
      return gen()*this->scale;
    }

  private:
    double scale;
    boost::gamma_distribution<> random;
  };

  //The timingModifier for geometric distributions
  class GeometricTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    GeometricTimingModifier(boost::shared_ptr<boost::mt19937> generator,double p,double minValue,double maxValue,bool isfixed){
      this->gen = generator;
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::geometric_distribution<> (p);
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::geometric_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::geometric_distribution<> random;
  };

  //The timingModifier for lognormal distributions
  class LognormalTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    LognormalTimingModifier(boost::shared_ptr<boost::mt19937> generator,double mean,double sigma,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::lognormal_distribution<> (mean,sigma);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::lognormal_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::lognormal_distribution<> random;
  };

  //The timingModifier for normal distributions
  class NormalTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    NormalTimingModifier(boost::shared_ptr<boost::mt19937> generator,double mean,double sigma,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::normal_distribution<> (mean, sigma);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::normal_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  public:
    boost::normal_distribution<> random;
  };

  //The timingModifier for poisson distributions
  class PoissonTimingModifier : public DistributionTimingModifier {

  public:
    //the regular constructor, it needs a random-distribution and a random-generator
    PoissonTimingModifier(boost::shared_ptr<boost::mt19937> generator,double mean,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::poisson_distribution<> (mean);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::poisson_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::poisson_distribution<> random;
  };

  //The timingModifier for triangle distributions
  class TriangleTimingModifier : public DistributionTimingModifier {

  public:
    TriangleTimingModifier(boost::shared_ptr<boost::mt19937> generator,double a,double b,double c,double minValue,double maxValue,bool isfixed){
      this->minValue = minValue;
      this->maxValue = maxValue;
      this->random = boost::triangle_distribution<> (a,b,c);
      this->gen = generator;
      this->isfixed = isfixed;
      this->factor = -1;
      this->hasBase = false;
    }

    virtual long double getFactor(){
      boost::variate_generator<boost::mt19937&,boost::triangle_distribution<> > gen(*(this->gen.get()),this->random);
      return gen();
    }

  private:
    boost::triangle_distribution<> random;
  };

} //namespace SystemC_VPC
#endif /* _INCLUDED_SYSTEMCVPC_TIMINGMODIFIER_HPP */
