#include <systemcvpc/Route.hpp>
namespace SystemC_VPC {
  size_t Route::createRouteId(){
    static size_t instanceCounter = 0;
    return ++instanceCounter; // start from 1
  }
}
