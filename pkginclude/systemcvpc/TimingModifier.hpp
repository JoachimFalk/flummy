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
				std::cout << "reRoll begin" << std::endl;
        while (1){
				  std::cout << "reRoll inner begin" << std::endl;
          double roled = getFactor();
          if ((roled >= minValue) && ((roled < maxValue) || (maxValue == -1))) {
            this->factor = roled+1;
            std::cout << "factor: " << this->factor-1 << std::endl;
            break;
          } else {
            std::cout << "nofactor: " << roled << std::endl;
          }  
				  std::cout << "reRoll inner end" << std::endl;
        }
				std::cout << "reRoll end" << std::endl;
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
      boost::shared_ptr<base_generator_type> generator;
			boost::shared_ptr<boost::mt19937> gen;
      std::vector<double> history;
      unsigned int position;
      double minValue;
      double maxValue;
  };

  class UniformRealTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      UniformRealTimingModifier(boost::uniform_real<> uniformRandom,boost::shared_ptr<base_generator_type> generator,double minValue, double maxValue){
	this->generator = generator;
	this->random = uniformRandom;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual void hello(){
        std::cout << "UniformTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }

      boost::uniform_real<> random;
  };

  class BernoulliTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      BernoulliTimingModifier(boost::bernoulli_distribution<> random,boost::shared_ptr<base_generator_type> generator,double minValue,double maxValue){
	this->generator = generator;
	this->random = random;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual void hello(){
        std::cout << "BernoulliTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }


      boost::bernoulli_distribution<> random;
  };

  class BinomialTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      BinomialTimingModifier(boost::binomial_distribution<> random,boost::shared_ptr<base_generator_type> generator,double minValue,double maxValue){
	this->generator = generator;
	this->random = random;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual void hello(){
        std::cout << "BinomialTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }

      boost::binomial_distribution<> random;
  };

  class CauchyTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      CauchyTimingModifier(boost::cauchy_distribution<> random,boost::shared_ptr<base_generator_type> generator,double minValue, double maxValue){
	this->generator = generator;
	this->random = random;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual void hello(){
        std::cout << "cauchyTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }

      boost::cauchy_distribution<> random;
  };

  class ExponentialTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      ExponentialTimingModifier(boost::exponential_distribution<> random,boost::shared_ptr<base_generator_type> generator,double minValue,double maxValue){
	this->generator = generator;
	this->random = random;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual void hello(){
        std::cout << "exponentialTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }

      boost::exponential_distribution<> random;
  };

  class GammaTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      GammaTimingModifier(boost::shared_ptr<boost::mt19937> generator,double shape,double scale,double minValue,double maxValue){
        this->gen = generator;
        this->minValue = minValue;
        this->maxValue = maxValue;
				this->random = boost::gamma_distribution<> (shape);
				this->scale = scale;  
				std::cout << scale << std::endl;
      }

      virtual void hello(){
        std::cout << "gammaTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
				std::cout << "scale start" << std::endl;
				boost::variate_generator<boost::mt19937&,boost::gamma_distribution<> > gen(*(this->gen.get()),this->random);
        return gen()*this->scale;
				std::cout << "scale end" << std::endl;
      }

      double scale;
      boost::gamma_distribution<> random;
  };

  class GeometricTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      GeometricTimingModifier(boost::geometric_distribution<> random,boost::shared_ptr<base_generator_type> generator,double minValue,double maxValue){
        this->generator = generator;
        this->random = random;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual void hello(){
        std::cout << "geometricTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }

      boost::geometric_distribution<> random;
  };

  class LognormalTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      LognormalTimingModifier(boost::lognormal_distribution<> random,boost::shared_ptr<base_generator_type> generator,double minValue,double maxValue){
	this->generator = generator;
	this->random = random;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }

      virtual void hello(){
        std::cout << "lognormalTimingModifier hello" << std::endl;
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
			boost::shared_ptr<boost::mt19937> gen;
  };

  class PoissonTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      PoissonTimingModifier(boost::poisson_distribution<> random,boost::shared_ptr<base_generator_type> generator,double minValue,double maxValue){
	this->generator = generator;
	this->random = random;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual void hello(){
        std::cout << "poissonTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }

      boost::poisson_distribution<> random;
  };

  class TriangleTimingModifier : public BaseTimingModifier {

    public:
      //the regular constructor, it needs a random-distribution and a random-generator
      TriangleTimingModifier(boost::triangle_distribution<> random,boost::shared_ptr<base_generator_type> generator,double minValue,double maxValue){
	this->generator = generator;
	this->random = random;
        this->minValue = minValue;
        this->maxValue = maxValue;
      }

      virtual void hello(){
        std::cout << "triangleTimingModifier hello" << std::endl;
      }

      virtual double getFactor(){
        return this->random(*(this->generator.get()));
      }

      boost::triangle_distribution<> random;
  };
} //namespace SystemC_VPC
#endif // HSCD_VPC_TIMINGMODIFIER_H_
