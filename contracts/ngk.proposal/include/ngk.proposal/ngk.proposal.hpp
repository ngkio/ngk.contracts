#pragma once

#include <ngk/binary_extension.hpp>
#include <ngk/ngk.hpp>
#include <ngk/ignore.hpp>
#include <ngk/transaction.hpp>

namespace ngk {

	using namespace ngk;
	using namespace std;

	class [[ngk::contract("ngk.proposal")]] propose : public ngk::contract {
      public:
         using contract::contract;
         static constexpr ngk::name active_permission{"active"_n};
         static constexpr ngk::name owner_permission{"owner"_n};

         [[ngk::action]]
         void addblacklist( name proposer, uint64_t proposal_id, name contract, uint8_t auto_exec,
               uint64_t expire_time);

         [[ngk::action]]
         void approve( uint64_t proposal_id, name account);

         [[ngk::action]]
         void cancel( uint64_t proposal_id, string reason );

         [[ngk::action]]
         void exec( uint64_t proposal_id);

      private:
   	  	 enum class proposal_type : uint8_t
   	  	 {
   	  	 	blacklist = 1
		 };
   	  	 enum class status : uint32_t
		 {
   	  	 	processing = 0,
   	  	 	success = 1,
   	  	 	canceled = 9,
		 };

         struct [[ngk::table]] proposal {
         	uint64_t                            proposal_id;
         	name 								proposer;
			uint64_t							expire_time;
			uint32_t 							min_approve_count;
			bool								auto_exec = false;
			uint32_t							type;
			std::string							value;
			uint32_t							status;
			uint64_t       						time;
			std::string							remark;
			std::vector<name>   				requested_approvals;
			std::vector<name>   				provided_approvals;

            uint64_t primary_key()const { return proposal_id; }
            uint64_t by_status()const { return status; }
         };

         typedef ngk::multi_index< "proposal"_n, proposal,
         							indexed_by<"status"_n, const_mem_fun<proposal, uint64_t, &proposal::by_status>>
				 					> proposals;

         struct approval {
         	name 				account;
            checksum256 		trx_id;
            uint64_t       		time;
         };

         struct [[ngk::table]] approvals_info {
         	uint64_t                proposal_id;
            std::vector<approval>   provided_approvals;

            uint64_t primary_key()const { return proposal_id; }
         };
         typedef ngk::multi_index< "approvals"_n, approvals_info > approvals;

         struct [[ngk::table]] theblacklist {
         	uint64_t    proposal_id;
            name        account;
//            bool		is_activated = false;

            uint64_t primary_key() const { return account.value; }
//            uint64_t by_activated() const { return is_activated ? 1 : 0; }
         };

         typedef ngk::multi_index< "theblacklist"_n, theblacklist
//				 					indexed_by<"activated"_n, const_mem_fun<theblacklist, uint64_t, &theblacklist::by_activated>>
         							> theblacklists;

	   	checksum256 get_trx_id()
	   	{
		   	checksum256 trx_id;
		   	auto tx_size = transaction_size();
		   	char tx[tx_size];
		   	auto read_size = read_transaction(tx, tx_size);
		   	check(tx_size == read_size, "read_transaction failed.");
		   	trx_id = sha256(tx, tx_size);
		   	return trx_id;
	   	}

	   	void activate(const proposal &prop);
   };
   /** @}*/ // end of @defgroup ngk.proposal
} /// namespace ngk
