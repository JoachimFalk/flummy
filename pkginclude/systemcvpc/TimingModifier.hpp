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
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
typedef boost::minstd_rand base_generator_type;

namespace SystemC_VPC{

  class TimingModifier
  {
		//TODO: Destructor machen!!!!
    public:

			//this constructor should result in an identity function
      TimingModifier(){
		  }
      TimingModifier(base_generator_type generator){
				this->uniformRandom = new boost::variate_generator<base_generator_type&, boost::uniform_real<> >(generator,boost::uniform_real<>(0.8,1.2));
      }
      virtual void modify(sc_core::sc_time dii, sc_core::sc_time latency){
       // base_generator_type generator(42);
       // boost::uniform_real<> uni_dist(0.8,1.2);
       // boost::variate_generator<base_generator_type&, boost::uniform_real<> > uni(generator, uni_dist);

        double factor = (*uniformRandom)();
        std::cout << "Timing dii: " << dii;
        std::cout << " lat: " << latency << std::endl;
        dii*=factor;
	latency *= factor;
        std::cout << "new Timing dii: " << dii;
        std::cout << " lat: " << latency << std::endl;

      }
			void hello(){ std::cout << "Alive and kicking";}

    private:
      boost::variate_generator<base_generator_type&, boost::uniform_real<> >* uniformRandom;
  };

} //namespace SystemC_VPC
#endif // HSCD_VPC_TIMINGMODIFIER_H_
