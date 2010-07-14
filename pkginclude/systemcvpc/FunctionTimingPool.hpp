#ifndef __INCLUDED__FUNCTION_TIMIG_POOL__H__
#define __INCLUDED__FUNCTION_TIMIG_POOL__H__

#include <map>
#include <boost/shared_ptr.hpp>

#include "FastLink.hpp"

namespace SystemC_VPC {

  /**
   * Internal helper class to manage  function specific delays.
   */
  class FunctionTiming{

  public:

    /**
     * \brief Default constuctor of an ComponentDealy instance
     * \param name specifies the name of the associated component
     * simulation
     */
    FunctionTiming( );

    /**
     * copy constructor
     */
    FunctionTiming( const FunctionTiming &delay );

    /**
     * \brief Adds a new function delay to the instance
     * \param funcname specifies the associated function
     * \param delay is the corresponding delay for the function execution
     */
    void addDelay(FunctionId fid, sc_time delay);

    /**
     * \brief Set the base delay to the instance
     * \param funcname specifies the associated function
     * \param delay is the corresponding delay for the function execution
     */
    void setBaseDelay(sc_time delay);
    sc_time getBaseDelay( ) const;

    /**
     * \brief Used to access delay
     * \param funcname specifies a possible function if given
     * \return delay required for a function execution  on the associated
     * component of the process. If no function name is given or there is
     * no corresponding entry registered the default delay is returned.
     */
    sc_time getDelay(FunctionId fid) const;

    /**
     * \brief Adds a new function latency to the instance
     * \param funcname specifies the associated function
     * \param latency is the corresponding latency for the function
     * execution
     */
    void addLatency(FunctionId fid, sc_time latency);

    /**
     * \brief Set the base latency to the instance
     * \param latency is the corresponding latency for the function
     * execution
     */
    void setBaseLatency(sc_time latency);
    sc_time getBaseLatency( ) const;

    /**
     * \brief Used to access latency
     * \param funcname specifies a possible function if given
     * \return latency required for a function execution  on the
     * associated component of the process. If no function name is given
     * or there is no corresponding entry registered the default latency
     * is returned.
     */
    sc_time getLatency(FunctionId fid) const;


    /**
     *
     */
    void setTiming(const Timing& timing);

  private:

    typedef std::vector<sc_time> FunctionTimes;
    // map of possible special delay depending on functions
    FunctionTimes funcDelays;

    // map of possible function specific latencies
    FunctionTimes funcLatencies;
  };

  typedef boost::shared_ptr<FunctionTiming> FunctionTimingPtr;
  typedef std::map<ProcessId, FunctionTimingPtr>  FunctionTimingPool;
  typedef boost::shared_ptr<FunctionTimingPool> FunctionTimingPoolPtr;

}
#endif // __INCLUDED__FUNCTION_TIMIG_POOL__H__
