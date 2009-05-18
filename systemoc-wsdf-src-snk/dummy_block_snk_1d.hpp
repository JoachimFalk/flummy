// vim: set sw=2 ts=8:

#ifndef INCLUDE_DUMMY_BLOCK_SNK_1D_HPP
#define INCLUDE_DUMMY_BLOCK_SNK_1D_HPP

#include <cstdlib>
#include <iostream>
#include <string>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

template <typename T = unsigned char>
class m_dummy_block_snk_1d: public smoc_actor {

private:

	const unsigned size_x;
	const unsigned size_y;

	T temp;

public:
	
	smoc_port_in<T> in;

private:

	void read_pixel(){
		for(unsigned counter_y = 0; counter_y < size_y; counter_y++){
			for(unsigned counter_x = 0; counter_x < size_x; counter_x++){
				temp = in[counter_y*size_x+counter_x];
			}
		}		
	}

	void store_image(){
		read_pixel();		
		std::cout << "Finished image " << cur_file << std::endl;
		cur_file++;
	}


	smoc_firing_state start;
	smoc_firing_state end;

	unsigned cur_file;
	

public:
	m_dummy_block_snk_1d( sc_module_name name ,
												unsigned size_x,
												unsigned size_y,
												unsigned num_files = 1)
		: smoc_actor(name,start),		
			size_x(size_x),
			size_y(size_y),
			cur_file(1)
	{
		start = in(size_x*size_y) 
			>> (VAR(this->cur_file) < num_files)
			>> CALL(m_dummy_block_snk_1d::store_image) 
			>> start
			| in(size_x*size_y)
			>> (VAR(this->cur_file) == num_files)
			>> CALL(m_dummy_block_snk_1d::store_image) 
			>> end;	
	}
	
};


#endif //INCLUDE_DUMMY_BLOCK_SNK_1D_HPP
