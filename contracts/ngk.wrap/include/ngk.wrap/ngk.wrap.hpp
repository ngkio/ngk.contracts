#pragma once

#include <ngk/ngk.hpp>
#include <ngk/ignore.hpp>
#include <ngk/transaction.hpp>

namespace ngk {
	using namespace ngk;
   /**
    * @defgroup ngkwrap ngk.wrap
    * @ingroup ngkcontracts
    * ngk.wrap contract simplifies Block Producer superuser actions by making them more readable and easier to audit.

    * It does not grant block producers any additional powers that do not already exist within the
    * system. Currently, 2/3 block producers can already change an account's keys or modify an
    * account's contract at the request of ECAF or an account's owner. However, the current method
    * is opaque and leaves undesirable side effects on specific system accounts.
    * ngk.wrap allows for a cleaner method of implementing these important governance actions.
    * @{
    */
   class [[ngk::contract("ngk.wrap")]] wrap : public contract {
      public:
         using contract::contract;

         /**
          * Execute action.
          *
          * Execute a transaction while bypassing regular authorization checks.
          *
          * @param executer - account executing the transaction,
          * @param trx - the transaction to be executed.
          *
          * @pre Requires authorization of ngk.wrap which needs to be a privileged account.
          *
          * @post Deferred transaction RAM usage is billed to 'executer'
          */
         [[ngk::action]]
         void exec( ignore<name> executer, ignore<transaction> trx );

         using exec_action = ngk::action_wrapper<"exec"_n, &wrap::exec>;
   };
   /** @}*/ // end of @defgroup ngkwrap ngk.wrap
} /// namespace ngk
