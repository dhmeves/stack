/*
 * IPC Manager
 *
 *    Vincenzo Maffione <v.maffione@nextworks.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __IPCM_H__
#define __IPCM_H__

#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <utility>

#include <librina/common.h>
#include <librina/ipc-manager.h>
#include <librina/patterns.h>

#include "rina-configuration.h"

//Addons
#include "addon.h"
#include "addons/console.h"
#include "addons/scripting.h"
//[+] Add more here...


//Constants
#define PROMISE_TIMEOUT_S 5
#define PROMISE_RETRY_NSEC 10000000 //1ms
#define _PROMISE_1_SEC_NSEC 1000000000

#ifndef FLUSH_LOG
	//Force log flushing
	#define FLUSH_LOG(_lev_, _ss_)\
			do{\
				LOGF_##_lev_ ("%s", (_ss_).str().c_str());\
				ss.str(string());\
			}while (0)
#endif //FLUSH_LOG

namespace rinad {

//
// Return codes
//
typedef enum ipcm_res{
	//Success
	IPCM_SUCCESS = 0,

	//Return value will be deferred
	IPCM_PENDING = 1,

	//Generic failure
	IPCM_FAILURE = -1,

	//TODO: add more codes here...
}ipcm_res_t;



//
// Promise base class
//
class Promise {

public:

	virtual ~Promise(){};

	//
	// Wait (blocking)
	//
	ipcm_res_t wait(void){
		unsigned int i;
		// Due to the async nature of the API, notifications (signal)
		// the transaction can well end before the thread is waiting
		// in the condition variable. As apposed to sempahores
		// pthread_cond don't keep the "credit"
		for(i=0; i < PROMISE_TIMEOUT_S *
				(_PROMISE_1_SEC_NSEC/ PROMISE_RETRY_NSEC) ;++i){
			try{
				if(ret != IPCM_PENDING)
					return ret;
				wait_cond.timedwait(0, PROMISE_RETRY_NSEC);
			}catch(...){};
		}

		//hard timeout expired
		ret = IPCM_FAILURE;
		return ret;
	};

	//
	// Signal the condition
	//
	inline void signal(void){
		wait_cond.signal();
	}

	//
	// Timed wait (blocking)
	//
	// Return SUCCESS or IPCM_PENDING if the operation did not finally
	// succeed
	//
	ipcm_res_t timed_wait(const unsigned int seconds){

		if(ret != IPCM_PENDING)
			return ret;
		try{
			wait_cond.timedwait(seconds, 0);
		}catch (rina::ConcurrentException& e) {
			if(ret != IPCM_PENDING)
				return ret;
			return IPCM_PENDING;
		};
		return ret;
	};

	//
	// Return code
	//
	ipcm_res_t ret;

protected:
	//Condition variable
	rina::ConditionVariable wait_cond;
};

//
// Create IPCP promise
//
class CreateIPCPPromise : public Promise {


public:
	int ipcp_id;
};

//
// Query RIB promise
//
class QueryRIBPromise : public Promise {

public:
	std::string serialized_rib;
};

//
// This base class encapsulates the generics of any two-step, so requiring
// interaction with the kernel, API call.
//
class TransactionState {

public:
	TransactionState(Promise* promise);
	virtual ~TransactionState(){};

	//
	// Used to recover the promise from the transaction
	//
	// @arg Template T: type of the promise
	//
	template<typename T>
	T* get_promise(){
		try{
			T* t = dynamic_cast<T*>(promise);
			return t;
		}catch(...){
			assert(0);
			return NULL;
		}
	}

	//
	// This method and signals any existing set complete flag
	//
	void completed(ipcm_res_t _ret){
		if(!promise)
			return;

		promise->ret = _ret;
		promise->signal();

	}

	//Promise
	Promise* promise;

	//Transaction id
	const int tid;

protected:
	TransactionState(Promise* promise_, const int tid_):
							promise(promise_),
							tid(tid_){
		if(promise)
			promise->ret = IPCM_PENDING;
	};
};

//
// Syscall specific transaction state
//
class SyscallTransState : public TransactionState {
public:
	SyscallTransState(Promise* promise_, const int tid_) :
					TransactionState(promise_, tid_){};
	virtual ~SyscallTransState(){};
};

//
// @brief The IPCManager class is in charge of managing the IPC processes
// life-cycle.
//
class IPCManager_ {

public:
	//
	// Initialize the IPCManager
	//
	void init(unsigned int wait_time, const std::string& loglevel);

	//
	// Start the script worker thread
	//
	ipcm_res_t start_script_worker();

	//
	// Start the console worker thread
	//
	ipcm_res_t start_console_worker();

	//
	// TODO: XXX?????
	//
	ipcm_res_t apply_configuration();


	//
	// List the existing IPCPs in the system
	//
	void list_ipcps(std::ostream& os);


	//
	// Get the IPCP ID given a difName
	// TODO: multiple IPCPs in the same DIF?
	//
	int get_ipcp_by_dif_name(std::string& difName);

	//
	// Checks if an IPCP exists by its ID
	//
	bool ipcp_exists(const unsigned short ipcp_id);

	//
	// List the available IPCP types
	//
	void list_ipcp_types(std::list<std::string>& types);


	//
	// Get the IPCP identifier where the application is registered
	// FIXME: return id instead?
	//
	bool lookup_dif_by_application(
		const rina::ApplicationProcessNamingInformation& apName,
		rina::ApplicationProcessNamingInformation& difName);


	//
	// Creates an IPCP process
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	//
	ipcm_res_t create_ipcp(CreateIPCPPromise* promise,
			const rina::ApplicationProcessNamingInformation& name,
			const std::string& type);

	//
	// Destroys an IPCP process
	//
	// This method is blocking
	//
	// @ret IPCM_SUCCESS on success IPCM_FAILURE
	//
	ipcm_res_t destroy_ipcp(const unsigned short ipcp_id);

	//
	// Assing an ipcp to a DIF
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	//
	ipcm_res_t assign_to_dif(Promise* promise, const unsigned short ipcp_id,
			  const rina::ApplicationProcessNamingInformation&
			  difName);

	//
	// Register an IPCP to a single DIF
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t register_at_dif(Promise* promise, const unsigned short ipcp_id,
			    const rina::ApplicationProcessNamingInformation&
			    difName);

	//
	// Enroll IPCP to a single DIF
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t enroll_to_dif(Promise* promise, const unsigned short ipcp_id,
			  const rinad::NeighborData& neighbor);

	//
	// Unregister app from an ipcp
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t unregister_app_from_ipcp(Promise* promise,
		const rina::ApplicationUnregistrationRequestEvent& req_event,
		const unsigned short slave_ipcp_id);

	//
	// Unregister an ipcp from another one
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t unregister_ipcp_from_ipcp(Promise* promise,
						const unsigned short ipcp_id,
						const unsigned short slave_ipcp_id);
	//
	// Update the DIF configuration
	//TODO: What is really this for?
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t update_dif_configuration(Promise* promise,
				const unsigned short ipcp_id,
				const rina::DIFConfiguration& dif_config);

	//
	// Retrieve the IPCP RIB in the form of a string
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t query_rib(QueryRIBPromise* promise, const unsigned short ipcp_id);

	//
	// Select a policy set
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t select_policy_set(Promise* promise, const unsigned short ipcp_id,
					const std::string& component_path,
					const std::string& policy_set);
	//
	// Set a policy "set-param"
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t set_policy_set_param(Promise* promise, const unsigned short ipcp_id,
						const std::string& path,
						const std::string& name,
						const std::string& value);
	//
	// Load policy plugin
	//
	// @param promise Promise object containing the future result of the
	// operation. The promise shall always be accessible until the
	// operation has been finished, so promise->ret value is different than
	// IPCM_PENDING.
	//
	// @ret IPCM_FAILURE on failure, otherwise the IPCM_PENDING
	ipcm_res_t plugin_load(Promise* promise, const unsigned short ipcp_id,
						const std::string& plugin_name,
						bool load);

	//
	// Get the current logging debug level
	//
	std::string get_log_level() const;

	//
	// Set the config
	//
	void loadConfig(rinad::RINAConfiguration& newConf){
		config = newConf;
	}

	//
	// Dump the current configuration
	// TODO return ostream or overload << operator instead
	//
	void dumpConfig(void){
		std::cout << config.toString() << std::endl;
	}

	//
	// Run the main I/O loop
	//
	void run(void);

	//
	// Stop I/O loop
	//
	inline void stop(void){
		keep_running = false;
	}

protected:

	//
	// Internal APIs
	//

	/**
	* Get the IPCP by DIF name
	*
 	* Recovers the IPCProcess pointer with the rwlock acquired to either
	* read lock or write lock
	*
	* @param read_lock When true, the IPCProcess instance is recovered with
	* the read lock acquired, otherwise the write lock is acquired.
	*/
	IPCMIPCProcess* select_ipcp_by_dif(const
			rina::ApplicationProcessNamingInformation& dif_name,
			bool write_lock=false);

	/**
	* Select a suitable IPCP
	* @param read_lock When true, the IPCProcess instance is recovered with
	* the read lock acquired, otherwise the write lock is acquired.
	*/
	IPCMIPCProcess* select_ipcp(bool write_lock=false);

	/**
	* Check application registration
	*/
	bool application_is_registered_to_ipcp(
			const rina::ApplicationProcessNamingInformation&,
			IPCMIPCProcess *slave_ipcp);
	/**
	* Get the IPCP by port id
	*
 	* Recovers the IPCProcess pointer with the rwlock acquired to either
	* read lock or write lock
	*
	* @param read_lock When true, the IPCProcess instance is recovered with
	* the read lock acquired, otherwise the write lock is acquired.
	*/
	IPCMIPCProcess* lookup_ipcp_by_port(unsigned int port_id,
						bool write_lock=false);

	/**
	* Collect flows for an application name
	*/
	void collect_flows_by_application(
			const rina::ApplicationProcessNamingInformation& app_name,
			std::list<rina::FlowInformation>& result);

	/**
	* Get the IPCP instance pointer
	*
	* Recovers the IPCProcess pointer with the rwlock acquired to either
	* read lock or write lock
	*
	* @param read_lock When true, the IPCProcess instance is recovered with
	* the read lock acquired, otherwise the write lock is acquired.
	*/
	IPCMIPCProcess* lookup_ipcp_by_id(const unsigned short id,
							bool write_lock=false);
	//
	// Internal event API
	//

	//Flow mgmt
	void flow_allocation_requested_event_handler(rina::FlowRequestEvent* event);
	void allocate_flow_response_event_handler( rina::AllocateFlowResponseEvent *event);
	void flow_deallocation_requested_event_handler(rina::FlowDeallocateRequestEvent* event);
	void flow_deallocated_event_handler(rina::FlowDeallocatedEvent* event);
	void ipcm_deallocate_flow_response_event_handler(rina::IpcmDeallocateFlowResponseEvent* event);
	void ipcm_allocate_flow_request_result_handler(rina::IpcmAllocateFlowRequestResultEvent* event);
	void application_flow_allocation_failed_notify(
						rina::FlowRequestEvent *event);
	void flow_allocation_requested_local(rina::FlowRequestEvent *event);

	void flow_allocation_requested_remote(rina::FlowRequestEvent *event);
	ipcm_res_t deallocate_flow(Promise* promise, const int ipcp_id,
			    const rina::FlowDeallocateRequestEvent& event);



	//Application registration mgmt
	void app_reg_req_handler(rina::ApplicationRegistrationRequestEvent *e);
	void app_reg_response_handler(rina::IpcmRegisterApplicationResponseEvent* e);
	void application_unregistration_request_event_handler(rina::ApplicationUnregistrationRequestEvent* event);
	void unreg_app_response_handler(rina::IpcmUnregisterApplicationResponseEvent *e);
	void notify_app_reg(
		const rina::ApplicationRegistrationRequestEvent& req_event,
		const rina::ApplicationProcessNamingInformation& app_name,
		const rina::ApplicationProcessNamingInformation& slave_dif_name,
		bool success);
	void application_manager_app_unregistered(
		rina::ApplicationUnregistrationRequestEvent event,
		int result);
	int ipcm_unregister_response_app(
			rina::IpcmUnregisterApplicationResponseEvent *event,
			IPCMIPCProcess * ipcp,
			rina::ApplicationUnregistrationRequestEvent& req);
	int ipcm_register_response_app(
		rina::IpcmRegisterApplicationResponseEvent *,
		IPCMIPCProcess * slave_ipcp,
		const rina::ApplicationRegistrationRequestEvent& req_event);

	//IPCP mgmt
	int ipcm_register_response_ipcp(
		rina::IpcmRegisterApplicationResponseEvent *event);
	int ipcm_unregister_response_ipcp(
				rina::IpcmUnregisterApplicationResponseEvent *event,
				TransactionState *trans);

	//DIF assignment mgmt
	void assign_to_dif_response_event_handler(rina::AssignToDIFResponseEvent * e);

	//DIF config mgmt
	void update_dif_config_response_event_handler(rina::UpdateDIFConfigurationResponseEvent* e);

	//Enrollment mgmt
	void enroll_to_dif_response_event_handler(rina::EnrollToDIFResponseEvent *e);

	//RIB queries
	void query_rib_response_event_handler(rina::QueryRIBResponseEvent *e);

	//Misc
	void os_process_finalized_handler(rina::OSProcessFinalizedEvent *event);
	void ipc_process_daemon_initialized_event_handler(
				rina::IPCProcessDaemonInitializedEvent *e);
	bool ipcm_register_response_common(
		rina::IpcmRegisterApplicationResponseEvent *event,
		const rina::ApplicationProcessNamingInformation& app_name,
		IPCMIPCProcess *slave_ipcp,
		const rina::ApplicationProcessNamingInformation& slave_dif_name);

	bool ipcm_register_response_common(
		rina::IpcmRegisterApplicationResponseEvent *event,
		const rina::ApplicationProcessNamingInformation& app_name,
		IPCMIPCProcess *slave_ipcp);


	bool ipcm_unregister_response_common(
				 rina::IpcmUnregisterApplicationResponseEvent *event,
				 IPCMIPCProcess *slave_ipcp,
				 const rina::ApplicationProcessNamingInformation&);

	//Plugins
	void ipc_process_set_policy_set_param_response_handler(
				rina::SetPolicySetParamResponseEvent *e);
	void ipc_process_plugin_load_response_handler(
				rina::PluginLoadResponseEvent *e);
	void ipc_process_select_policy_set_response_handler(
					rina::SelectPolicySetResponseEvent *e);

	/*
	* Get the transaction state. Template parameter is the type of the
	* specific state required for the type of transaction
	*
	* @ret A pointer to the state or NULL
	* @warning This method does NOT throw exceptions.
	*/
	template<typename T>
	T* get_transaction_state(int tid){
		T* t;
		TransactionState* state;

		//Read lock
		rina::ReadScopedLock readlock(trans_rwlock);

		if ( pend_transactions.find(tid) != pend_transactions.end() ) {
			state = pend_transactions[tid];
			assert(state->tid == tid);
			try{
				t = dynamic_cast<T*>(state);
				return t;
			}catch(...){
				assert(0);
				return NULL;
			}
		}
		return NULL;
	}

	/*
	* Add the transaction state. Template parameter is the type of the
	* specific state required for the type of transaction
	*
	* @ret 0 if success -1 otherwise.
	*/
	int add_transaction_state(TransactionState* t);

	/*
	* @Remove transaction state.
	*
	* Remove the transaction state from the pending transactions db. The
	* method does NOT free (delete) the transaction pointer.
	*
	* @ret 0 if success -1 otherwise.
	*/
	int remove_transaction_state(int tid);

	/*
	* This map encapsulates the existing transactions and their state
	*
	* The state of the transaction includes the input *and* the output
	* parameters for the specific operation that was started.
	*
	* key: transaction_id value: transaction state
	*/
	std::map<int, TransactionState*> pend_transactions;

	//TODO unify syscalls and non-syscall state

	/**
	* Get syscall transaction state (waiting for a notification)
	*/
	SyscallTransState* get_syscall_transaction_state(int tid){
		//Read lock
		rina::ReadScopedLock readlock(trans_rwlock);

		if ( pend_sys_trans.find(tid) !=  pend_sys_trans.end() )
			return pend_sys_trans[tid];

		return NULL;
	};

	/**
	* Add syscall transaction state (waiting for a notification)
	*
	* @ret 0 if success -1 otherwise.
	*/
	int add_syscall_transaction_state(SyscallTransState* t);

	/*
	* Remove syscall transaction state
	*
	* @ret 0 if success -1 otherwise.
	*/
	int remove_syscall_transaction_state(int tid);

	/**
	* Pending syscall states
	*
	* key: ipcp ID value: transaction state
	*/
	std::map<int, SyscallTransState*> pend_sys_trans;

	//Rwlock for transactions
	rina::ReadWriteLockable trans_rwlock;

	// RINA configuration internal state
	rinad::RINAConfiguration config;

	//Script thread
	rina::Thread *script;

	//IPCM Console instance
	IPCMConsole *console;

	//TODO: map of addons

	//Current logging level
	std::string log_level_;

	//Keep running flag
	volatile bool keep_running;

	IPCMIPCProcessFactory ipcp_factory_;

public:
	//Generator of opaque identifiers
	rina::ConsecutiveUnsignedIntegerGenerator __tid_gen;
private:
	//Singleton
	IPCManager_();
	virtual ~IPCManager_();
	friend class Singleton<rinad::IPCManager_>;
};


//Singleton instance
extern Singleton<rinad::IPCManager_> IPCManager;

}//rinad namespace

#endif  /* __IPCM_H__ */
