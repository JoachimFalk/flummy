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

#ifndef HSCD_VPC_TIMINGMODIFIER_H_
#define HSCD_VPC_TIMINGMODIFIER_H_

#include <systemc>
#include <vector>
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
typedef boost::minstd_rand base_generator_type;

namespace SystemC_VPC{

  class TimingModifier
  {
    public:
      //fake function for reRolling the random times
      virtual void reRoll(){
      }

      virtual sc_core::sc_time rePlay(sc_core::sc_time time){
        return time;
      }

      virtual void hello(){
        std::cout << "TimingModifier hello" << std::endl;
      }

      //fake function for the modifcation
      //Parameter: the time to modify
      //return: the modified time
      virtual sc_core::sc_time modify(sc_core::sc_time time){
        return time;
      }

      //this constructor should result in an identity function
      TimingModifier(){
      }

      virtual void reset(){
      }

      sc_core::sc_time replay(sc_core::sc_time time){
        return time;
      }
  };
  class BaseTimingModifier : public TimingModifier {
    public:
      virtual void hello(){
        std::cout << "BaseTimingModifier hello" << std::endl;
      }

      virtual sc_core::sc_time modify(sc_core::sc_time time){
        this->reRoll();
        std::cout << "(roled) old: " << time << " modified: " << time*this->factor << std::endl;
        this->history.push_back(this->factor);
        this->position++;
        return time*this->factor;
      }
      //generates a new random factor used later for modification 

      virtual void reRoll(){
        while (1){
          double roled = getFactor();
          if ((roled >= minValue) && ((roled < maxValue) || (maxValue == -1))) {
            this->factor = roled+1;
            std::cout << "factor: " << this->factor-1 << std::endl;
            break;
          } else {
            std::cout << "nofactor: " << roled << std::endl;
          }  
        }
      }

      virtual double getFactor(){
        return 1;
      }

      virtual void reset(){
        this->position = 0;
        this->history.clear();
        std::cout << "reset" << std::endl;
      }

      virtual sc_core::sc_time rePlay(sc_core::sc_time time){
        this->position++;
        std::cout << "(replay) old: " << time << " modified: " << time*(this->history[(this->position)-1]) << std::endl;
        return time*(this->history[(this->position)-1]);
      }

      //shared pointers were used to ensure that the factor is shared between all copys of this instance 
      boost::uniform_real<> random;
      double factor;
			boost::shared_ptr<boost::mt19937> gen;
      std::vector<double> history;
      unsigned int position;
      double minValue;
      double maxValue;
  };

  class UniformRealTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      UniformRealTimingModifier(boost::shared_ptr<boost::mt19937> generator,double min,double max,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::uniform_real<> (min,max);
        this->gen = generator;
      }

      virtual void hello(){
        std::cout << "UniformTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::uniform_real<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::uniform_real<> random;
  };

  class BernoulliTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      BernoulliTimingModifier(boost::shared_ptr<boost::mt19937> generator,double p,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::bernoulli_distribution<> (p);
        this->gen = generator;
      }

      virtual void hello(){
        std::cout << "BernoulliTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::bernoulli_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }


      boost::bernoulli_distribution<> random;
  };

  class BinomialTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      BinomialTimingModifier(boost::shared_ptr<boost::mt19937> generator,int t,double p,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::binomial_distribution<> (t,p);
        this->gen = generator;
      }

      virtual void hello(){
        std::cout << "BinomialTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::binomial_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::binomial_distribution<> random;
  };

  class CauchyTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      CauchyTimingModifier(boost::shared_ptr<boost::mt19937> generator,double median,double sigma,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::cauchy_distribution<> (median,sigma);
        this->gen = generator;
      }

      virtual void hello(){
        std::cout << "cauchyTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::cauchy_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::cauchy_distribution<> random;
  };

  class ExponentialTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      ExponentialTimingModifier(boost::shared_ptr<boost::mt19937> generator,double lambda,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::exponential_distribution<> (lambda);
        this->gen = generator;
      }

      virtual void hello(){
        std::cout << "exponentialTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::exponential_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::exponential_distribution<> random;
  };

  class GammaTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      GammaTimingModifier(boost::shared_ptr<boost::mt19937> generator,double shape,double scale,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::gamma_distribution<> (shape);
        this->gen = generator;
				this->scale = scale;  
      }

      virtual void hello(){
        std::cout << "gammaTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::gamma_distribution<> > gen(*(this->gen.get()),this->random);
        return gen()*this->scale;
      }

      double scale;
      boost::gamma_distribution<> random;
  };

  class GeometricTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      GeometricTimingModifier(boost::shared_ptr<boost::mt19937> generator,double p,double minValue,double maxValue){
        this->gen = generator;
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::geometric_distribution<> (p);
      }

      virtual void hello(){
        std::cout << "geometricTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::geometric_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::geometric_distribution<> random;
  };

  class LognormalTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      LognormalTimingModifier(boost::shared_ptr<boost::mt19937> generator,double mean,double sigma,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::lognormal_distribution<> (mean,sigma);
        this->gen = generator;
      }

      virtual void hello(){
        std::cout << "lognormalTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::lognormal_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::lognormal_distribution<> random;
  };

  class NormalTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      NormalTimingModifier(boost::shared_ptr<boost::mt19937> generator,double mean,double sigma,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::normal_distribution<> (mean, sigma);
				this->gen = generator;
      }

      virtual void hello(){
        std::cout << "normalTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::normal_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::normal_distribution<> random;
  };

  class PoissonTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      PoissonTimingModifier(boost::shared_ptr<boost::mt19937> generator,double mean,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::poisson_distribution<> (mean);
				this->gen = generator;
      }

      virtual void hello(){
        std::cout << "poissonTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::poisson_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::poisson_distribution<> random;
  };

  class TriangleTimingModifier : public BaseTimingModifier {

    public:
      TriangleTimingModifier(boost::shared_ptr<boost::mt19937> generator,double a,double b,double c,double minValue,double maxValue){
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::triangle_distribution<> (a,b,c);
				this->gen = generator;
      }

      virtual void hello(){
        std::cout << "triangleTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				boost::variate_generator<boost::mt19937&,boost::triangle_distribution<> > gen(*(this->gen.get()),this->random);
        return gen();
      }

      boost::triangle_distribution<> random;
  };
} //namespace SystemC_VPC
#endif // HSCD_VPC_TIMINGMODIFIER_H_
