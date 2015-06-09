//
// RIB v2 test suite
//
//	  Marc Sune <marc.sune (at) bisdn.de>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301  USA
//
#include <iostream>
#include <stdlib.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "librina/cdap.h"
#include "librina/ipc-process.h"
#include "librina/common.h"
#include "librina/cdap_v2.h"
#include "librina/rib_v2.h"

using namespace rina;
using namespace rina::rib;
using namespace rina::cdap;
using namespace rina::cdap_rib;

//fwd decl
class MyObj;

//RIB daemon proxy
static rina::rib::RIBDaemonProxy* ribd = NULL;
//Application Entity name
static const std::string ae ="ae_name";
//Version
cdap_rib::vers_info_t version;
//RIB handle
static rib_handle_t handle = 1234;
static rib_handle_t handle2;


class ribBasicOps : public CppUnit::TestFixture {

	CPPUNIT_TEST_SUITE( ribBasicOps );

	CPPUNIT_TEST( testInit );
	CPPUNIT_TEST( testSchemaCreation );
	CPPUNIT_TEST( testRIBCreation );
	CPPUNIT_TEST( testAssociation );
	CPPUNIT_TEST( testAddObj );
	CPPUNIT_TEST( testConnect );
	CPPUNIT_TEST( testOperations);
	CPPUNIT_TEST( testDisconnect );
	CPPUNIT_TEST( testRemoveObj );
	CPPUNIT_TEST( testDeassociation );
	CPPUNIT_TEST( testRIBDestruction );
	CPPUNIT_TEST( testFini );

	CPPUNIT_TEST_SUITE_END();

private:


public:
	void setUp();
	void tearDown();

	void testInit();
	void testSchemaCreation();
	void testRIBCreation();
	void testAssociation();
	void testAddObj();
	void testConnect();
	void testOperations();
	void testDisconnect();
	void testRemoveObj();
	void testDeassociation();
	void testRIBDestruction();
	void testFini();
};

//Register
CPPUNIT_TEST_SUITE_REGISTRATION( ribBasicOps );

class remoteHandlers : public rina::rib::RIBOpsRespHandlers {

//
//Required mock ups
//
public:
	~remoteHandlers(){};

	void remoteCreateResult(const cdap_rib::con_handle_t &con,
			const cdap_rib::obj_info_t &obj,
			const cdap_rib::res_info_t &res){ (void)con; (void)obj; (void)res; };
	void remoteDeleteResult(const cdap_rib::con_handle_t &con,
			const cdap_rib::res_info_t &res){ (void)con; (void)res; };
	void remoteReadResult(const cdap_rib::con_handle_t &con,
			const cdap_rib::obj_info_t &obj,
			const cdap_rib::res_info_t &res){ (void)con; (void)obj; (void)res; };
	void remoteCancelReadResult(const cdap_rib::con_handle_t &con,
			const cdap_rib::res_info_t &res){ (void)con; (void)res; };
	void remoteWriteResult(const cdap_rib::con_handle_t &con,
			const cdap_rib::obj_info_t &obj,
			const cdap_rib::res_info_t &res){ (void)con; (void)obj; (void)res; };
	void remoteStartResult(const cdap_rib::con_handle_t &con,
			const cdap_rib::obj_info_t &obj,
			const cdap_rib::res_info_t &res){ (void)con; (void)obj; (void)res; };
	void remoteStopResult(const cdap_rib::con_handle_t &con,
			const cdap_rib::obj_info_t &obj,
			const cdap_rib::res_info_t &res){ (void)con; (void)obj; (void)res; };
};

static class remoteHandlers remote_handlers;

class AppHandlers : public rina::cacep::AppConHandlerInterface {

public:
	~AppHandlers(){};

	/// A remote Connect request has been received.
	void connect(int invoke_id, const cdap_rib::con_handle_t &con){ (void)con; (void)invoke_id;};
	/// A remote Connect response has been received.
	void connectResult(const cdap_rib::res_info_t &res,
			const cdap_rib::con_handle_t &con){ (void)con; (void)res; };
	/// A remote Release request has been received.
	void release(int invoke_id, const cdap_rib::con_handle_t &con){ (void)con; (void)invoke_id; };
	/// A remote Release response has been received.
	void releaseResult(const cdap_rib::res_info_t &res,
			const cdap_rib::con_handle_t &con){ (void)con; (void)res; };
};

static class AppHandlers app_handlers;

//CDAP provider mockup
class CDAPProviderMockup : public CDAPProviderInterface {

public:
	virtual ~CDAPProviderMockup(){};

	virtual cdap_rib::con_handle_t remote_open_connection(
			const cdap_rib::vers_info_t &ver,
			const cdap_rib::src_info_t &src,
			const cdap_rib::dest_info_t &dest,
			const cdap_rib::auth_info &auth, int port_id){
		 cdap_rib::con_handle_t con;
		 return con;
	};

	//remote
	virtual int remote_close_connection(unsigned int port){ return 0;};
	virtual int remote_create(unsigned int port,
				  const cdap_rib::obj_info_t &obj,
				  const cdap_rib::flags_t &flags,
				  const cdap_rib::filt_info_t &filt){ return 0;};
	virtual int remote_delete(unsigned int port,
				  const cdap_rib::obj_info_t &obj,
				  const cdap_rib::flags_t &flags,
				  const cdap_rib::filt_info_t &filt){ return 0;};
	virtual int remote_read(unsigned int port,
				const cdap_rib::obj_info_t &obj,
				const cdap_rib::flags_t &flags,
				const cdap_rib::filt_info_t &filt){ return 0;};
	virtual int remote_cancel_read(unsigned int port,
				       const cdap_rib::flags_t &flags,
				       int invoke_id){ return 0;};
	virtual int remote_write(unsigned int port,
				 const cdap_rib::obj_info_t &obj,
				 const cdap_rib::flags_t &flags,
				 const cdap_rib::filt_info_t &filt){ return 0;};
	virtual int remote_start(unsigned int port,
				 const cdap_rib::obj_info_t &obj,
				 const cdap_rib::flags_t &flags,
				 const cdap_rib::filt_info_t &filt){ return 0;};
	virtual int remote_stop(unsigned int port,
				const cdap_rib::obj_info_t &obj,
				const cdap_rib::flags_t &flags,
				const cdap_rib::filt_info_t &filt){ return 0;};

	//local
	virtual void send_open_connection_result(const cdap_rib::con_handle_t &con,
					      const cdap_rib::res_info_t &res,
					      int invoke_id){};
	virtual void send_close_connection_result(unsigned int port,
					       const cdap_rib::flags_t &flags,
					       const cdap_rib::res_info_t &res,
					       int invoke_id){};
	virtual void send_create_result(unsigned int port,
					    const cdap_rib::obj_info_t &obj,
					    const cdap_rib::flags_t &flags,
					    const cdap_rib::res_info_t &res,
					    int invoke_id){};
	virtual void send_delete_result(unsigned int port,
					    const cdap_rib::obj_info_t &obj,
					    const cdap_rib::flags_t &flags,
					    const cdap_rib::res_info_t &res,
					    int invoke_id){};
	virtual void send_read_result(unsigned int port,
					  const cdap_rib::obj_info_t &obj,
					  const cdap_rib::flags_t &flags,
					  const cdap_rib::res_info_t &res,
					  int invoke_id){

		switch(invoke_id){
			case 4:
				CPPUNIT_ASSERT_MESSAGE("READ result for invoke id 4 != SUCCESS", res.code_ == cdap_rib::CDAP_SUCCESS);
				break;

			default:
				CPPUNIT_ASSERT_MESSAGE("READ result for an invalid invoke id", 0);
				break;
		}

	};
	virtual void send_cancel_read_result(
			unsigned int port, const cdap_rib::flags_t &flags,
			const cdap_rib::res_info_t &res, int invoke_id){};
	virtual void send_write_result(unsigned int port,
					   const cdap_rib::flags_t &flags,
					   const cdap_rib::res_info_t &res,
					   int invoke_id){
		switch(invoke_id){
			case 2:
			case 3:
				CPPUNIT_ASSERT_MESSAGE("WRITE result for invoke id != OPERATION NOT SUPPORTED", res.code_ == cdap_rib::CDAP_OP_NOT_SUPPORTED);
				break;
			default:
				CPPUNIT_ASSERT_MESSAGE("WRITE result for an invalid invoke id", 0);
				break;
		}

	};
	virtual void send_start_result(unsigned int port,
					   const cdap_rib::obj_info_t &obj,
					   const cdap_rib::flags_t &flags,
					   const cdap_rib::res_info_t &res,
					   int invoke_id){};
	virtual void send_stop_result(unsigned int port,
					  const cdap_rib::flags_t &flags,
					  const cdap_rib::res_info_t &res,
					  int invoke_id){};
	virtual void process_message(cdap_rib::SerializedObject &message,
				     unsigned int port){};

	virtual void destroy_session(int port){ (void)port; /*FIXME*/ };
};

static CDAPProviderMockup cdap_provider_mockup;
static cdap::CDAPCallbackInterface* rib_provider = NULL;
//
//
//

class MyObjEncoder: public Encoder<uint32_t> {

public:
	virtual ~MyObjEncoder(){}

	void encode(const uint32_t &obj, cdap_rib::ser_obj_t& serobj){
		//TODO fill in
		(void)obj;
		(void)serobj;
	};

	virtual void decode(const cdap_rib::ser_obj_t &serobj,
							uint32_t& des_obj){
		//TODO fill in
		(void)serobj;
		(void)des_obj;
	};
};

//A type
class MyObj : public RIBObj<uint32_t> {

public:
	MyObj(uint32_t initial_value) : RIBObj<uint32_t>(initial_value){};
	virtual ~MyObj(){};

	const std::string& get_class() const{
		return class_;
	}

	AbstractEncoder* get_encoder(){
		return &encoder;
	};

	void read(const cdap_rib::con_handle_t &con,
					const std::string& fqn,
					const std::string& class_,
					const cdap_rib::filt_info_t &filt,
					const int invoke_id,
					cdap_rib::SerializedObject &obj_reply,
					cdap_rib::res_info_t& res){


		CPPUNIT_ASSERT_MESSAGE("Invalid invoke id", invoke_id>=4);
		res.code_ = cdap_rib::CDAP_SUCCESS;
	};



	MyObjEncoder encoder;
	static const std::string class_;
};

const std::string MyObj::class_ = "MyObj";

//Another type
class OtherObj : public RIBObj<uint32_t> {

public:
	OtherObj(uint32_t initial_value) : RIBObj<uint32_t>(initial_value){};
	virtual ~OtherObj(){};

	const std::string& get_class() const{
		return class_;
	}

	AbstractEncoder* get_encoder(){
		return &encoder;
	};

	void read(const cdap_rib::con_handle_t &con,
					const std::string& fqn,
					const std::string& class_,
					const cdap_rib::filt_info_t &filt,
					const int invoke_id,
					cdap_rib::SerializedObject &obj_reply,
					cdap_rib::res_info_t& res){
	};



	MyObjEncoder encoder;
	static const std::string class_;
};

const std::string OtherObj::class_ = "OtherObj";



///
/// Schema's create callbacks (mockup)
///
static bool callback_1_called=false;
void create_callback_1(const rib_handle_t rib,
				const cdap_rib::con_handle_t &con,
				const std::string& fqn,
				const std::string& class_,
				const cdap_rib::filt_info_t &filt,
				const int invoke_id,
				const cdap_rib::SerializedObject &obj_req,
				cdap_rib::SerializedObject &obj_reply,
				cdap_rib::res_info_t& res){
	CPPUNIT_ASSERT_MESSAGE("Invalid invoke id during create generic", invoke_id==7);
	res.code_ = cdap_rib::CDAP_SUCCESS;
	callback_1_called = true;
}
static bool callback_2_called=false;
void create_callback_2(const rib_handle_t rib,
				const cdap_rib::con_handle_t &con,
				const std::string& fqn,
				const std::string& class_,
				const cdap_rib::filt_info_t &filt,
				const int invoke_id,
				const cdap_rib::SerializedObject &obj_req,
				cdap_rib::SerializedObject &obj_reply,
				cdap_rib::res_info_t& res){
	CPPUNIT_ASSERT_MESSAGE("Invalid invoke id during create specific", invoke_id==6);
	res.code_ = cdap_rib::CDAP_SUCCESS;
	callback_2_called = true;
}

//
// End of mockups
//

//Objects
MyObj *obj1, *obj2, *obj3;
OtherObj *obj4;
std::string name1 = "/x";
std::string name2 = "/y";
std::string name3 = "/x/z";
std::string name4 = "/x/z/t";
std::string name_other = "/x/other";
std::string name_create = "/x/z/t/c";
int64_t inst_id1, inst_id2, inst_id3, inst_id4, inst_id5;

//Setups

//fwd decl
namespace rina{namespace rib{
void __set_cdap_provider(cdap::CDAPProviderInterface* p);
cdap::CDAPCallbackInterface* __get_rib_provider(void);
}} //namespaces fwd decl


void ribBasicOps::setUp(){

}

void ribBasicOps::tearDown(){

}

void ribBasicOps::testInit(){
	rina::rib::RIBDaemonProxy* ribd_;
	cdap_rib::cdap_params params;
	params.is_IPCP_ = false;
	ribd = NULL;

	//Before init getting a proxy should throw an exception
	try{
		ribd = rina::rib::RIBDaemonProxyFactory();
	}catch(...){
		printf("Captured exception\n");
	}

	CPPUNIT_ASSERT_MESSAGE("Invalid ribdproxy before init", ribd == NULL);

	rina::rib::init(&app_handlers, &remote_handlers, params);

	//Mockups
	rina::rib::__set_cdap_provider(&cdap_provider_mockup);
	rib_provider = rina::rib::__get_rib_provider();

	CPPUNIT_ASSERT_MESSAGE("Invalid RIB provider", rib_provider != NULL);

	//Double call to init should throw an exception
	try{
		rina::rib::init(&app_handlers, &remote_handlers, params);
		CPPUNIT_ASSERT_MESSAGE("Double call to init did not throw an exception", 0);
	}catch(...){}

	//Create the proxy
	ribd = rina::rib::RIBDaemonProxyFactory();
	CPPUNIT_ASSERT_MESSAGE("Unable to recover a valid proxy", ribd != NULL);

	ribd_ = rina::rib::RIBDaemonProxyFactory();
	CPPUNIT_ASSERT_MESSAGE("Unable to recover a second valid proxy", ribd_ != NULL);
	delete ribd_;
}

void ribBasicOps::testSchemaCreation(){

	version.version_ = 0x1;

	//Create a schema (empty)
	try{
		ribd->createSchema(version);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception throw during the creation of a schema", 0);
	}

	//try to recreate
	try{
		ribd->createSchema(version);
		CPPUNIT_ASSERT_MESSAGE("Exception not thrown during creation of a duplicate schema", 0);
	}catch(eSchemaExists& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during creation of a dubplicate schema", 0);
	}

	//List versions and check if it exists
	std::list<cdap_rib::vers_info_t> vers;
	try{
		vers = ribd->listVersions();
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during listVersions method", 0);
	}

	//Check the result
	CPPUNIT_ASSERT_MESSAGE("Invalid list of versions length", vers.size() == 1);
	if(vers.size() == 1){
		CPPUNIT_ASSERT_MESSAGE("Schema has not been created with the right version or listVersions() has a bug", version.version_ == (*vers.begin()).version_);
	}


	//Register with an invalid version
	try{
		cdap_rib::vers_info_t wrong_version;
		wrong_version.version_ = 0x2;
		ribd->addCreateCallbackSchema(wrong_version,
						MyObj::class_,
						name1,
						create_callback_1);
		CPPUNIT_ASSERT_MESSAGE("Exception not thrown during registration of create callback with an invalid version", 0);
	}catch(eSchemaNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during registration of create callback with an invalid version", 0);
	}


	//Register an invalid class name
	try{
		std::string class_ = "";
		ribd->addCreateCallbackSchema(version,
						class_,
						name1,
						create_callback_1);
		CPPUNIT_ASSERT_MESSAGE("Exception not thrown during registration of create callback with an invalid class name", 0);
	}catch(eSchemaInvalidClass& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during registration of create callback with an invalid class name", 0);
	}

	//Register a valid specific path
	try{
		ribd->addCreateCallbackSchema(version,
						MyObj::class_,
						name_create,
						create_callback_2);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during registration of create callback with a specific name", 0);
	}

	//Re-register
	try{
		ribd->addCreateCallbackSchema(version,
						MyObj::class_,
						name_create,
						create_callback_2);
		CPPUNIT_ASSERT_MESSAGE("Exception not thrown during re-registration of create callback (specific)", 0);
	}catch(eSchemaCBRegExists& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during re-registration of create callback (specific)", 0);
	}

	//Register non-specific
	try{
		ribd->addCreateCallbackSchema(version,
						MyObj::class_,
						"",
						create_callback_1);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during registration of create callback (generic)", 0);
	}
	//Re-register
	try{
		ribd->addCreateCallbackSchema(version,
						MyObj::class_,
						"",
						create_callback_1);
		CPPUNIT_ASSERT_MESSAGE("Exception not thrown during re-registration of create callback (generic)", 0);
	}catch(eSchemaCBRegExists& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during re-registration of create callback (generic)", 0);
	}


}

void ribBasicOps::testRIBCreation(){

	version.version_ = 0x1;

	//Get a handle of an inexistent RIB
	try{
		handle = ribd->get(version, ae);
		CPPUNIT_ASSERT_MESSAGE("Got an invalid handle for an inexistent RIB", 0);
	}catch(eRIBNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during get invalid RIB handle", 0);
	}


	//Create a RIB with an invalid schema
	try{
		cdap_rib::vers_info_t wrong_version;
		wrong_version.version_ = 0x11111;

		handle = ribd->createRIB(wrong_version);

		CPPUNIT_ASSERT_MESSAGE("Exception not thrown during creation of a RIB of an invalid version", 0);
	}catch(eSchemaNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid Exception thrown during creation of a RIB of an invalid version", 0);
	}

	//Create a valid RIB
	try{
		handle = ribd->createRIB(version);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during creation of a RIB of a valid version", 0);
	}

	//Check handle
	CPPUNIT_ASSERT_MESSAGE("Invalid handle during valid RIB creation", handle == 1);

	//Create another valid RIB
	try{
		handle2 = ribd->createRIB(version);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during creation of a RIB of a valid version", 0);
	}

	//Check handle
	CPPUNIT_ASSERT_MESSAGE("Invalid handle during valid RIB creation", handle2 == 2);
	CPPUNIT_ASSERT_MESSAGE("Invalid handle during valid RIB creation", handle2 != handle);

}

void ribBasicOps::testAssociation(){

	rib_handle_t tmp;
	rib_handle_t wrong_handle = 9999;

	//Associate an invalid RIB handle
	//Destroy an inexistent schema (not yet implemented)
	try{
		ribd->associateRIBtoAE(wrong_handle, ae);
		CPPUNIT_ASSERT_MESSAGE("Associate to with an invalid RIB handle succeeded", 0);
	}catch(eRIBNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during associate with an invalid RIB handle", 0);
	}

	//Perform a valid association
	try{
		ribd->associateRIBtoAE(handle, ae);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception throw during associate with an valid RIB handle", 0);
	}

	//Check get with a valid version but invalid ae
	try{
		std::string wrong_ae = "fff";
		tmp = ribd->get(version, wrong_ae);
	}catch(eRIBNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during get() with a valid version but invalid ae_name", 0);
	}

	//Check get with a valid an invalid version but valid ae
	try{
		cdap_rib::vers_info_t wrong_version;
		wrong_version.version_ = 0x1234;
		tmp = ribd->get(wrong_version, ae);
	}catch(eRIBNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during get() with an invalid version but a valid ae_name", 0);
	}

	//Check get with a valid version and ae
	try{
		tmp = ribd->get(version, ae);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception throw during get() with a valid version and ae_name", 0);
	}
	CPPUNIT_ASSERT_MESSAGE("Bug in get(); invalid handle returned", tmp == handle);

	//Retry with the same handle
	try{
		ribd->associateRIBtoAE(handle, ae);
		CPPUNIT_ASSERT_MESSAGE("Double associate with a valid RIB handle succeeded", 0);
	}catch(eRIBAlreadyAssociated& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during double associate with a valid RIB handle", 0);
	}

	//Retry with another handle (same AE+version)
	try{
		ribd->associateRIBtoAE(handle2, ae);
		CPPUNIT_ASSERT_MESSAGE("Associate with a valid RIB handle but with the same version (already registered) succeeded", 0);
	}catch(eRIBAlreadyAssociated& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during associate with a valid RIB handle but with the same version (already registered) succeeded", 0);
	}
}

void ribBasicOps::testAddObj(){

	MyObj* tmp;
	rib_handle_t wrong_handle = 9999;
	std::string invalid_name1 = "";
	std::string invalid_name2 = "x";
	std::string invalid_name3 = "/x/";

	obj1 = new MyObj(1);
	obj2 = new MyObj(2);
	obj3 = new MyObj(3);
	obj4 = new OtherObj(4);

	//Attempt to add them into an invalid RIB handle
	try{
		ribd->addObjRIB(wrong_handle, name1, &obj1);
		CPPUNIT_ASSERT_MESSAGE("Add obj to with an invalid RIB handle succeeded", 0);
	}catch(eRIBNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during Add obj with an invalid RIB handle", 0);
	}

	//Attempt to add them into a valid RIB handle but with invalid names
	try{
		ribd->addObjRIB(handle, invalid_name1, &obj1);
		CPPUNIT_ASSERT_MESSAGE("Add obj to with an invalid name to RIB handle succeeded", 0);
	}catch(eObjInvalidName& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during Add obj with an invalid name to RIB handle", 0);
	}
	try{
		ribd->addObjRIB(handle, invalid_name2, &obj1);
		CPPUNIT_ASSERT_MESSAGE("Add obj to with an invalid name to RIB handle succeeded", 0);
	}catch(eObjInvalidName& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during Add obj with an invalid name to RIB handle", 0);
	}
	try{
		ribd->addObjRIB(handle, invalid_name3, &obj1);
		CPPUNIT_ASSERT_MESSAGE("Add obj to with an invalid name to RIB handle succeeded", 0);
	}catch(eObjInvalidName& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during Add obj with an invalid name to RIB handle", 0);
	}

	//Add the right object (/x)
	try{
		tmp = obj1;
		inst_id1 = ribd->addObjRIB(handle, name1, &obj1);
		CPPUNIT_ASSERT_MESSAGE("Did not set to null obj1", obj1 == NULL);
		CPPUNIT_ASSERT_MESSAGE("Invalid instance id for obj1", inst_id1 == 1);
		obj1 = tmp;
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Could not add obj with in a valid RIB", 0);
	}

	//Check that inst_id matches
	CPPUNIT_ASSERT_MESSAGE("Insertion id and getObjInstId() mismatch", inst_id1 == ribd->getObjInstId(handle, name1));

	//Check that utils work
	try{
		std::string ts;
		CPPUNIT_ASSERT_MESSAGE("Insertion id and getObjInstId() mismatch", inst_id1 == ribd->getObjInstId(handle, name1));
		CPPUNIT_ASSERT_MESSAGE("Class validation in getObjInstId() fails", inst_id1 == ribd->getObjInstId(handle, name1, MyObj::class_));
		ts = ribd->getObjFqn(handle, inst_id1);
		CPPUNIT_ASSERT_MESSAGE("FQN mismatch with getObjFqn()", name1 == ts);
		ts = ribd->getObjFqn(handle, inst_id1, MyObj::class_);
		CPPUNIT_ASSERT_MESSAGE("Class validation in getObjFqn() fails", name1 == ts);
		ts = ribd->getObjClass(handle, inst_id1);
		CPPUNIT_ASSERT_MESSAGE("Class of getObjClass() mismatch", ts.compare(MyObj::class_) == 0);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during utils API validation", 0);
	}

	tmp = obj2;
	try{
		ribd->addObjRIB(handle, name1, &obj2);
		CPPUNIT_ASSERT_MESSAGE("Add overlapping object to RIB succeeded", 0);
	}catch(eObjExists& e){
		CPPUNIT_ASSERT_MESSAGE("Add overlapping object failed, but modified the object pointer",  tmp == obj2);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception thrown during Add obj with an overlapping object", 0);
	}

	//Add another valid object (/y)
	try{
		tmp = obj2;
		inst_id2 = ribd->addObjRIB(handle, name2, &obj2);
		CPPUNIT_ASSERT_MESSAGE("Did not set to null obj2", obj2 == NULL);
		CPPUNIT_ASSERT_MESSAGE("Invalid instance id for obj2", inst_id2 == 2);
		obj2 = tmp;
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Could not add obj2 with in a valid RIB", 0);
	}

	//Check that obj2 is the child of obj1 (it shouldn't)
	std::string parent_fqn;
	try{
		parent_fqn = ribd->getObjParentFqn(handle, name2);
		CPPUNIT_ASSERT_MESSAGE("Parent incorrectly match", parent_fqn != name1);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during child validation", 0);
	}
	try{
		int64_t p_id = ribd->getObjInstId(handle, parent_fqn);
	}catch(eObjDoesNotExist& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception thrown during getObjInstId()", 0);
	}

	//Retry overlap should fail
	tmp = obj2;
	try{
		ribd->addObjRIB(handle, name2, &obj2);
		CPPUNIT_ASSERT_MESSAGE("Add overlapping object to RIB succeeded", 0);
	}catch(eObjExists& e){
		CPPUNIT_ASSERT_MESSAGE("Add overlapping object failed, but modified the object pointer",  tmp == obj2);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception thrown during Add obj with an overlapping object", 0);
	}

	//Add an inner object (/x/z)
	try{
		tmp = obj3;
		inst_id3 = ribd->addObjRIB(handle, name3, &obj3);
		CPPUNIT_ASSERT_MESSAGE("Did not set to null obj3", obj3 == NULL);
		CPPUNIT_ASSERT_MESSAGE("Invalid instance id for obj3", inst_id3 == 3);
		obj3 = tmp;
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during Add obj 3", 0);
	}


	//Check that obj3 is the child of obj1 (it should!)
	try{
		parent_fqn = ribd->getObjParentFqn(handle, name3);
		CPPUNIT_ASSERT_MESSAGE("Parent mismatch", parent_fqn == name1);
		CPPUNIT_ASSERT_MESSAGE("Parent mismatch", parent_fqn != name2);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during child validation", 0);
	}
	try{
		int64_t p_id = ribd->getObjInstId(handle, parent_fqn);
		CPPUNIT_ASSERT_MESSAGE("Parent id mismatch", p_id == inst_id1);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception thrown during getObjInstId() obj 3", 0);
	}


	//Add an inner object (/x/other)
	try{
		OtherObj* tmp2 = obj4;
		inst_id4 = ribd->addObjRIB(handle, name_other, &obj4);
		CPPUNIT_ASSERT_MESSAGE("Did not set to null obj4", obj4 == NULL);
		CPPUNIT_ASSERT_MESSAGE("Invalid instance id for obj4", inst_id4 == 4);
		obj4 = tmp2;
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during Add obj 4", 0);
	}
}

//////// CLIENT /////////////////////
cdap_rib::con_handle_t con_ok, con_ko;
cdap_rib::flags_t flags_ok;

void ribBasicOps::testConnect(){

	//Fill in the necessary stuff (only)
	con_ok.port_ = 0x1;
	con_ok.dest_.ae_name_ = ae;
	con_ok.version_ = version;

	//Fill in the necessary
	con_ko.port_ = 0x1;
	con_ko.dest_.ae_name_ = "invalid_ae";
	con_ko.version_ = version;

	//Invalid AE name
	try{
		rib_provider->open_connection(con_ko, flags_ok, 0x1);
		CPPUNIT_ASSERT_MESSAGE("Connection with an invalid AE name has succeeded", 0);
	}catch(eRIBNotFound&){
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception thrown during open_connection", 0);
	}

	//Fake stablishment of a connection
	try{
		rib_provider->open_connection(con_ok, flags_ok, 0x2);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during open_connection", 0);
	}

	//Retry; this should succeed (overwrite)
	try{
		rib_provider->open_connection(con_ok, flags_ok, 0x3);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during open_connection (retry)", 0);
	}
}

#define PREFIX_MESSAGE 0x11000000

void ribBasicOps::testOperations(){

	obj_info_t obj_info1;
	cdap_rib::filt_info_t filter;
	int* message;
	int invoke_id;

	//Use FQN first
	obj_info1.inst_ = -1;
	obj_info1.name_ = "/x/y/z/invalid";
	obj_info1.value_.size_ = sizeof(int);
	obj_info1.value_.message_ = new int;
	message = (int*)obj_info1.value_.message_;

	//Issue a request with an invalid con id
	invoke_id = 1;
	try{
		(*message) = PREFIX_MESSAGE | invoke_id;
		rib_provider->read_request(con_ko, obj_info1, filter, invoke_id);
		CPPUNIT_ASSERT_MESSAGE("READ operation with an invalid connection name has succeeded", 0);
	}catch(...){

	}

	//Issue a request to a valid object but with an invalid operation
	invoke_id = 2;
	obj_info1.name_ = name1;
	try{
		(*message) = PREFIX_MESSAGE | invoke_id;
		rib_provider->write_request(con_ok, obj_info1, filter, invoke_id);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during read_req", 0);
	}

	//Invalid operation
	invoke_id = 3;
	obj_info1.name_ = name1;
	try{
		(*message) = PREFIX_MESSAGE | invoke_id;
		rib_provider->write_request(con_ok, obj_info1, filter, invoke_id);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during read_req", 0);
	}

	//Issue a request to a valid object and a valid operation
	invoke_id = 4;
	obj_info1.name_ = name1;
	try{
		(*message) = PREFIX_MESSAGE | invoke_id;
		rib_provider->read_request(con_ok, obj_info1, filter, invoke_id);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during valid read_req", 0);
	}

	//Issue a create request for a class that exists but in valid object and a valid operation
	invoke_id = 5;
	obj_info1.name_ = name_create;
	obj_info1.class_ = OtherObj::class_;
	try{
		(*message) = PREFIX_MESSAGE | invoke_id;
		rib_provider->create_request(con_ok, obj_info1, filter, invoke_id);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during \"valid\" create_req", 0);
	}

	//Issue a create request to specific  => callback 2
	invoke_id = 6;
	obj_info1.name_ = name_create;
	obj_info1.class_ = MyObj::class_;
	try{
		(*message) = PREFIX_MESSAGE | invoke_id;
		rib_provider->create_request(con_ok, obj_info1, filter, invoke_id);
		CPPUNIT_ASSERT_MESSAGE("Specific create callback not called", callback_2_called == true);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during valid create_req", 0);
	}

	//Issue a create request to other path => generic callback 1
	invoke_id = 7;
	obj_info1.name_ = "/h";
	obj_info1.class_ = MyObj::class_;
	try{
		(*message) = PREFIX_MESSAGE | invoke_id;
		rib_provider->create_request(con_ok, obj_info1, filter, invoke_id);
		CPPUNIT_ASSERT_MESSAGE("Generic create callback not called", callback_1_called == true);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during valid create_req", 0);
	}
}


void ribBasicOps::testDisconnect(){
	//TODO
}
//////// CLIENT /////////////////////

void ribBasicOps::testRemoveObj(){

	rib_handle_t wrong_handle = 9999;
	int64_t wrong_inst_id = 99999;

	// Invalid RIB
	try{
		ribd->removeObjRIB(wrong_handle, inst_id1);
		CPPUNIT_ASSERT_MESSAGE("Remove object with an invalid RIB handle succeeded", 0);
	}catch(eRIBNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during invalid handle remove obj", 0);
	}

	// Invalid inst id
	try{
		ribd->removeObjRIB(handle, wrong_inst_id);
		CPPUNIT_ASSERT_MESSAGE("Remove inexistent inst_id succeeded", 0);
	}catch(eObjDoesNotExist& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception thrown during Add obj with an overlapping object", 0);
	}

	//Remove obj1 which has obj3 as a child should fail
	try{
		ribd->removeObjRIB(handle, inst_id1);
		CPPUNIT_ASSERT_MESSAGE("Remove child object (inst_id3) succeeded", 0);
	}catch(eObjHasChildren& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception thrown during remove obj with children", 0);
	}

	//Remove obj2
	try{
		ribd->removeObjRIB(handle, inst_id2);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during remove obj inst2", 0);
	}

	//Remove obj2
	try{
		ribd->removeObjRIB(handle, inst_id2);
	}catch(eObjDoesNotExist& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception thrown during remove obj inst2 not existing anymore", 0);
	}

	//Remove obj3
	try{
		ribd->removeObjRIB(handle, inst_id3);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during remove obj inst3", 0);
	}

	//Remove obj4
	try{
		ribd->removeObjRIB(handle, inst_id4);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during remove obj inst4", 0);
	}
	//Now remove obj1
	try{
		ribd->removeObjRIB(handle, inst_id1);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception thrown during remove obj inst1", 0);
	}
}

void ribBasicOps::testDeassociation(){

	rib_handle_t wrong_handle = 9999;

	//Deassociate with an invalid RIB handle
	try{
		ribd->deassociateRIBfromAE(wrong_handle, ae);
		CPPUNIT_ASSERT_MESSAGE("Deassociate to with an invalid RIB handle succeeded", 0);
	}catch(eRIBNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during deassociate with an invalid RIB handle", 0);
	}

	//Deassociate with an valid RIB handle but not associated
	try{
		ribd->deassociateRIBfromAE(handle2, ae);
		CPPUNIT_ASSERT_MESSAGE("Deassociate to with a valid not associated RIB handle succeeded", 0);
	}catch(eRIBNotAssociated& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during deassociate with a valid not associated RIB handle", 0);
	}

	//Deassociate with an invalid ae
	try{
		std::string wrong_ae = "fff";
		ribd->deassociateRIBfromAE(wrong_handle, wrong_ae);
		CPPUNIT_ASSERT_MESSAGE("Deassociate to with an invalid RIB handle succeeded", 0);
	}catch(eRIBNotFound& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during deassociate with an invalid AE name", 0);
	}

	//Deassociate from an existing association relation
	try{
		ribd->deassociateRIBfromAE(handle, ae);
	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Exception throw during deassociate with a valid RIB handle and version", 0);
	}

	//Try it again
	try{
		ribd->deassociateRIBfromAE(handle, ae);
		CPPUNIT_ASSERT_MESSAGE("Deassociate to with an invalid RIB handle succeeded (retry). State not properly cleaned?", 0);
	}catch(eRIBNotAssociated& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Invalid exception throw during deassociate with an invalid RIB handle(retry). State not properly cleaned?", 0);
	}

}

void ribBasicOps::testRIBDestruction(){

	cdap_rib::vers_info_t wrong_version;
	wrong_version.version_ = 0x99999;
	rib_handle_t wrong_handle = 9999;

	//Destroy an inexistent schema (not yet implemented)
	try{
		ribd->destroySchema(wrong_version);
		CPPUNIT_ASSERT_MESSAGE("Destroy invalid schema succeeded", 0);
	}catch(eNotImplemented& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during destroy invalid schema", 0);
	}

	//Destroy an inexistent RIB (not yet implemented)
	try{
		ribd->destroyRIB(wrong_handle);
		CPPUNIT_ASSERT_MESSAGE("Destroy invalid RIB succeeded", 0);
	}catch(eNotImplemented& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during destroy invalid RIB", 0);
	}

	//Destroy an existent schema (not yet implemented)
	try{
		ribd->destroySchema(version);
		CPPUNIT_ASSERT_MESSAGE("Destroy a valid schema succeeded", 0);
	}catch(eNotImplemented& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during destroy of a valid schema", 0);
	}

	//Destroy an inexistent RIB (not yet implemented)
	try{
		ribd->destroyRIB(handle);
		CPPUNIT_ASSERT_MESSAGE("Destroy a valid RIB succeeded", 0);
	}catch(eNotImplemented& e){

	}catch(...){
		CPPUNIT_ASSERT_MESSAGE("Wrong exception thrown during destroy a valid RIB", 0);
	}
}

void ribBasicOps::testFini(){
	rina::rib::fini();
}

int main(int args, char** argv){

	(void)args;
	(void)argv;

	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest( registry.makeTest() );
	bool wasSuccessful = runner.run( "", false );

	int rc = (wasSuccessful) ? EXIT_SUCCESS : EXIT_FAILURE;
	return rc;
}
