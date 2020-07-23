#include <ngk/action.hpp>
#include <ngk/crypto.hpp>
#include <ngk/permission.hpp>

#include <ngk.proposal/ngk.system.hpp>
#include <ngk.proposal/ngk.proposal.hpp>

namespace ngk {

void propose::addblacklist( name proposer, uint64_t proposal_id, name contract, uint8_t auto_exec,
			uint64_t expire_time)
{
   	require_auth( proposer );
   	uint64_t now = current_time_point().elapsed.count() / 1000;

	ngk::system sys("ngk"_n, get_first_receiver(), get_datastream());
	sys.check_account_contract(contract, "account not a contract.");
	std::vector<name> requested = sys.get_top_producers(21);

	auto itr = std::find(requested.begin(), requested.end(), proposer);
	check(itr != requested.end(), "proposer should be one of the BPs.");
   	check( expire_time >= now, "proposal expired" );

   	proposals proptable( get_self(), get_self().value );
   	check( proptable.find( proposal_id ) == proptable.end(), "proposal with the same name exists" );

	theblacklists blacklisttable( get_self(), get_self().value );
	check( blacklisttable.find( contract.value ) == blacklisttable.end(), "this contract has been proposed" );

   	proptable.emplace( get_self(), [&]( auto& prop ) {
	  	prop.proposal_id       		= proposal_id;
	  	prop.proposer  				= proposer;
		prop.expire_time			= expire_time;
		prop.min_approve_count  	= 15;
		prop.auto_exec 				= auto_exec > 0;
		prop.type					= (uint8_t)propose::proposal_type::blacklist;
		prop.value					= contract.to_string();
		prop.status					= (uint32_t)propose::status::processing;
		prop.time					= now;
		prop.requested_approvals	= requested;
		prop.provided_approvals.reserve(requested.size());
		prop.provided_approvals.push_back(proposer);
   	});

   	approvals apptable( get_self(), get_self().value );
   	apptable.emplace( get_self(), [&]( auto& a ) {
      	a.proposal_id       = proposal_id;
      	a.provided_approvals.reserve( requested.size() );
		a.provided_approvals.push_back( propose::approval {.account = proposer, .trx_id = propose::get_trx_id(), .time = now});
   	});

   	//send delay transaction
	const string reason = "expired cancel";
	auto delay_sec = (expire_time - now) / 1000;
	delay_sec = delay_sec < 10 ? 10 : delay_sec;

	ngk::transaction out;
	out.actions.emplace_back(permission_level{ get_self(), propose::active_permission },
			get_self(), "cancel"_n,
			std::make_tuple(proposal_id, reason)
	);
	ngk::cancel_deferred(proposal_id);
	out.delay_sec = delay_sec;
	out.send(proposal_id, get_self(), true);
}

void propose::approve( uint64_t proposal_id, name account)
{
   	require_auth( account );
	uint64_t now = current_time_point().elapsed.count() / 1000;

	proposals proptable( get_self(), get_self().value );
	auto prop_itr = proptable.find( proposal_id );
	check( prop_itr != proptable.end(), "proposal doesn't exists" );
	check( prop_itr->status == (uint32_t)propose::status::processing, "proposal can't be approved" );
	check( prop_itr->expire_time >= now, "proposal expired" );

	auto requested_itr = std::find(prop_itr->requested_approvals.begin(), prop_itr->requested_approvals.end(), account);
	check( requested_itr != prop_itr->requested_approvals.end(), "approval is not on the list of requested approvals" );

	auto is_active = prop_itr->provided_approvals.size() + 1 >= prop_itr->min_approve_count && prop_itr->auto_exec;

	proptable.modify( prop_itr, get_self(), [&]( auto& prop ) {
		prop.provided_approvals.push_back( account );
		prop.requested_approvals.erase( requested_itr );
		if(is_active) prop.status = (uint32_t)propose::status::success;
	});

   	approvals apptable( get_self(), get_self().value );
   	auto apps_it = apptable.find(proposal_id);
	apptable.modify( apps_it, get_self(), [&]( auto& a ) {
		a.provided_approvals.push_back( propose::approval {.account = account, .trx_id = propose::get_trx_id(), .time = now} );
	});

	if(is_active)
	{
		propose::activate(*prop_itr);
	}
}

void propose::cancel( uint64_t proposal_id, string reason )
{
	proposals proptable( get_self(), get_self().value );
	auto prop_itr = proptable.find( proposal_id );
	check( prop_itr != proptable.end(), "proposal doesn't exists" );
	check( prop_itr->status == (uint32_t)propose::status::processing, "proposal can't be canceled" );

	check(has_auth(prop_itr->proposer) || has_auth(get_self()), "has no permission");

	proptable.modify( prop_itr, get_self(), [&]( auto& prop ) {
		prop.status = (uint32_t)propose::status::canceled;
		prop.remark = reason;
	});
}

void propose::exec( uint64_t proposal_id)
{
	proposals proptable( get_self(), get_self().value );
	auto prop_itr = proptable.find( proposal_id );
	check( prop_itr != proptable.end(), "proposal doesn't exists" );
	check( prop_itr->status == (uint32_t)propose::status::processing, "proposal can't be canceled" );

	require_auth( prop_itr->proposer );

	check(prop_itr->provided_approvals.size() >= prop_itr->min_approve_count, "current approval not enough.");

	proptable.modify( prop_itr, get_self(), [&]( auto& prop ) {
		prop.status = (uint32_t)propose::status::success;
	});

	propose::activate(*prop_itr);
}

void propose::activate(const proposal &prop)
{
	ngk::cancel_deferred(prop.proposal_id);

	check(prop.type == (uint8_t)propose::proposal_type::blacklist, "current proposal type not support.");

	name contract = name(prop.value);
	theblacklists blacklisttable( get_self(), get_self().value );
	blacklisttable.emplace( get_self(), [&]( auto& bl ) {
		bl.proposal_id 		= prop.proposal_id;
		bl.account 			= contract;
	});

	//update account abi
	action(
			permission_level{contract, propose::active_permission},
			"ngk"_n,
			"setabi"_n,
			std::make_tuple(contract, std::vector<char>{}))
			.send();
	//update account permission
	const auto owner_auth = ngk::authority{.threshold = 1,
			.keys = {{.key = ngk::public_key{}, .weight = 1}},
			.accounts = {},
			.waits = {}};
	const auto active_auth = ngk::authority{.threshold = 1,
			.keys = {{.key = ngk::public_key{}, .weight = 1}},
			.accounts = {},
			.waits = {}};

	action(
			permission_level{contract, propose::active_permission},
			"ngk"_n,
			"updateauth"_n,
			std::make_tuple(contract, propose::active_permission, propose::owner_permission, active_auth))
			.send();

	action(
			permission_level{contract, propose::owner_permission},
			"ngk"_n,
			"updateauth"_n,
			std::make_tuple(contract, propose::owner_permission, name{}, owner_auth))
			.send();
}

} /// namespace ngk
