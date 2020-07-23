#include <ngk.system/native.hpp>

#include <ngk/check.hpp>

namespace ngksystem {

   void native::onerror( ignore<uint128_t>, ignore<std::vector<char>> ) {
      ngk::check( false, "the onerror action cannot be called directly" );
   }

}
