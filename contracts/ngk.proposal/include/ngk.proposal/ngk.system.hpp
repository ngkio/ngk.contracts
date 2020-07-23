#include <ngk/ngk.hpp>
#include <ngk/binary_extension.hpp>
#include <ngk/privileged.hpp>

namespace ngk
{
	using namespace ngk;
	class[[ngk::contract("ngk.system")]] system: public ngk::contract
	{
	public:
		system(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds), producerstable(receiver, receiver.value), abitable(receiver, receiver.value) {}

		inline void check_account_contract(name account, std::string message)
		{
			auto itr = abitable.find(account.value);
			check(itr != abitable.end(), message);
		}

		inline std::vector<name> get_top_producers(uint32_t top_count)
		{
			auto idx = producerstable.get_index<"prototalvote"_n>();
			auto itr = idx.begin();
			std::vector<name> result;

			for (int i = 0; i < top_count && itr != idx.end(); ++i,itr++)
			{
				result.push_back(itr->owner);
			}

			return result;
		}

	private:

		struct [[ngk::table, ngk::contract("ngk.system")]] producer_info {
			name                                                     owner;
			double                                                   total_votes = 0;
			ngk::public_key                                        producer_key; /// a packed public key object
			bool                                                     is_active = true;

			uint64_t primary_key()const { return owner.value;                             }
			double   by_votes()const    { return is_active ? -total_votes : total_votes;  }
		};

		typedef ngk::multi_index< "producers"_n, producer_info,
				indexed_by<"prototalvote"_n, const_mem_fun<producer_info, double, &producer_info::by_votes>  >
				> producers_table;



		struct [[ngk::table("abihash"), ngk::contract("ngk.system")]] abi_hash {
			name              owner;
			checksum256       hash;
			uint64_t primary_key()const { return owner.value; }

			NGKLIB_SERIALIZE( abi_hash, (owner)(hash) )
		};

		typedef multi_index< "abihash"_n, abi_hash > abi_hash_table;

		abi_hash_table abitable;
		producers_table producerstable;
	};



	struct permission_level_weight
	{
		permission_level permission;
		uint16_t weight;
	};

	struct wait_weight
	{
		uint32_t wait_sec;
		uint16_t weight;
	};

	struct authority
	{
		uint32_t threshold = 0;
		std::vector<key_weight> keys;
		std::vector<permission_level_weight> accounts;
		std::vector<wait_weight> waits;
	};
}