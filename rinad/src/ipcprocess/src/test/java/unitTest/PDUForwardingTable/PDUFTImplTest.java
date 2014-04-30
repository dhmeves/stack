package unitTest.PDUForwardingTable;


import java.util.ArrayList;

import junit.framework.Assert;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.junit.Before;
import org.junit.Test;

import rina.PDUForwardingTable.api.FlowStateObject;
import rina.PDUForwardingTable.api.FlowStateObjectGroup;
import rina.cdap.api.message.CDAPMessage;
import rina.cdap.api.message.ObjectValue;
import rina.encoding.impl.EncoderImpl;
import rina.encoding.impl.googleprotobuf.flowstate.FlowStateEncoder;
import rina.encoding.impl.googleprotobuf.flowstate.FlowStateGroupEncoder;
import rina.ipcprocess.api.IPCProcess;
import rina.ipcprocess.impl.PDUForwardingTable.PDUFTImpl;
import rina.ipcprocess.impl.PDUForwardingTable.routingalgorithms.dijkstra.DijkstraAlgorithm;
import rina.ipcprocess.impl.PDUForwardingTable.routingalgorithms.dijkstra.Vertex;
import unitTest.PDUForwardingTable.fakeobjects.FakeCDAPSessionManager;
import unitTest.PDUForwardingTable.fakeobjects.FakeIPCProcess;
import unitTest.PDUForwardingTable.fakeobjects.FakeRIBDaemon;

public class PDUFTImplTest {

	private static final Log log = LogFactory.getLog(PDUFTImplTest.class);
	
	protected PDUFTImpl impl = null;
	
	static {
		System.loadLibrary("rina_java");
	}
	
	@Before
	public void set()
	{
		impl = new PDUFTImpl();
		impl.setTest(true);
	}

	@Test
	public void PDUFTImpl_Constructor_True()
	{
		log.info("PDUFTImpl_Constructor_True()");
		Assert.assertEquals(impl.getClass(), PDUFTImpl.class);
	}
	
	@Test
	public void flowdeAllocated_DeAllocateFlow_True()
	{
		log.info("flowdeAllocated_DeAllocateFlow_True()");
		impl.setIPCProcess(new FakeIPCProcess(new FakeCDAPSessionManager(), new FakeRIBDaemon(), new FlowStateEncoder()));
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		impl.flowAllocated(1, 1, 2, 1);
		
		Assert.assertTrue(impl.flowDeallocated(1));
	}

	@Test
	public void flowdeAllocated_DeAllocateFlow_False()
	{
		log.info("flowdeAllocated_DeAllocateFlow_False()");
		impl.setIPCProcess(new FakeIPCProcess(new FakeCDAPSessionManager(), new FakeRIBDaemon(), new FlowStateEncoder()));
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		impl.flowAllocated(1, 1, 2, 1);
		
		Assert.assertFalse(impl.flowDeallocated(2));
	}
	
	@Test
	public void enrollmentToNeighbor_NoFlowAllocateds_Null()
	{
		log.info("enrollmentToNeighbor_NoFlowAllocateds_Null()");
		FakeRIBDaemon rib = new FakeRIBDaemon();
		EncoderImpl encoder = new EncoderImpl();
        encoder.addEncoder(FlowStateObject.class.getName(), new FlowStateEncoder());
        encoder.addEncoder(FlowStateObjectGroup.class.getName(), new FlowStateGroupEncoder());
		IPCProcess ipc = new FakeIPCProcess(new FakeCDAPSessionManager(), rib, encoder);
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		impl.enrollmentToNeighbor(3, true, 2);
		
		Assert.assertNull(rib.recoveredObjectGroup);
	}

	@Test
	public void enrollmentToNeighbor_SendMessageRecoverObject_True()
	{
		log.info("enrollmentToNeighbor_SendMessageRecoverObject_True()");
		FakeRIBDaemon rib = new FakeRIBDaemon();
		IPCProcess ipc = new FakeIPCProcess(new FakeCDAPSessionManager(), rib, new FlowStateGroupEncoder());
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		impl.flowAllocated(1, 1, 2, 1);
	
		impl.enrollmentToNeighbor(3, true, 2);
		
		Assert.assertEquals(rib.recoveredObjectGroup.getFlowStateObjectArray().get(0).getAddress(), 1);
		Assert.assertEquals(rib.recoveredObjectGroup.getFlowStateObjectArray().get(0).getPortid(), 1);
		Assert.assertEquals(rib.recoveredObjectGroup.getFlowStateObjectArray().get(0).getNeighborAddress(), 2);
		Assert.assertEquals(rib.recoveredObjectGroup.getFlowStateObjectArray().get(0).getNeighborPortid(), 1);
		
		
	}
	
	
	@Test
	public void propagateFSDB_EmptyDB_True()
	{
		log.info("propagateFSDB_EmptyDB_True()");
		FakeRIBDaemon rib = new FakeRIBDaemon();
		IPCProcess ipc = new FakeIPCProcess(new FakeCDAPSessionManager(), rib, new FlowStateGroupEncoder());
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		Assert.assertFalse(impl.propagateFSDB());;
	}
	
	@Test
	public void propagateFSDB_NewFlow_True()
	{
		FakeRIBDaemon rib = new FakeRIBDaemon();
		IPCProcess ipc = new FakeIPCProcess(new FakeCDAPSessionManager(), rib, new FlowStateGroupEncoder());
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		impl.flowAllocated(1, 1, 2, 1);
		
		Assert.assertTrue(impl.propagateFSDB());
	}
	
	@Test
	public void propagateFSDB_SendObject_NotNull()
	{
		FakeRIBDaemon rib = new FakeRIBDaemon();
		IPCProcess ipc = new FakeIPCProcess(new FakeCDAPSessionManager(), rib, new FlowStateGroupEncoder());
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		impl.flowAllocated(1, 1, 2, 1);
		impl.propagateFSDB();
		
		Assert.assertNotNull(rib.recoveredObjectGroup);
	}
	
	@Test
	public void propagateFSDB_SendObjectSameObject_True()
	{
		FakeRIBDaemon rib = new FakeRIBDaemon();
		IPCProcess ipc = new FakeIPCProcess(new FakeCDAPSessionManager(), rib, new FlowStateGroupEncoder());
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		impl.flowAllocated(1, 1, 2, 1);
		impl.propagateFSDB();
		
		Assert.assertEquals(rib.recoveredObjectGroup.getFlowStateObjectArray().get(0).getAddress(), 1);
		Assert.assertEquals(rib.recoveredObjectGroup.getFlowStateObjectArray().get(0).getPortid(), 1);
		Assert.assertEquals(rib.recoveredObjectGroup.getFlowStateObjectArray().get(0).getNeighborAddress(), 2);
		Assert.assertEquals(rib.recoveredObjectGroup.getFlowStateObjectArray().get(0).getNeighborPortid(), 1);
	}
	
	@Test
	public void propagateFSDB_PropagateTwoTimes_NULL()
	{
		FakeRIBDaemon rib = new FakeRIBDaemon();
		IPCProcess ipc = new FakeIPCProcess(new FakeCDAPSessionManager(), rib, new FlowStateGroupEncoder());
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		
		impl.flowAllocated(1, 1, 2, 1);
		impl.propagateFSDB();
		rib.recoveredObjectGroup = null;
		impl.propagateFSDB();
		
		Assert.assertNull(rib.recoveredObjectGroup);
	}
	
	@Test
	public void writeMessageRecieved_writeGroup_True()
	{
		ObjectValue objectValue = new ObjectValue();
		FlowStateGroupEncoder fse = new FlowStateGroupEncoder();
		FakeCDAPSessionManager cdapSessionManager = new FakeCDAPSessionManager();
		FakeRIBDaemon rib = new FakeRIBDaemon();
		FlowStateObject obj1 = new FlowStateObject(2,1,3,1, true, 1, 0);
		ArrayList<FlowStateObject> fsoL = new ArrayList<FlowStateObject>();
		fsoL.add(obj1);
		IPCProcess ipc = new FakeIPCProcess(cdapSessionManager, rib, fse);
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		FlowStateObjectGroup fsg = new FlowStateObjectGroup(fsoL);

		try
		{
			objectValue.setByteval(fse.encode(fsg));
			CDAPMessage cdapMessage = cdapSessionManager.getWriteObjectRequestMessage(1, null,
				null, FlowStateObjectGroup.FLOW_STATE_GROUP_RIB_OBJECT_CLASS, 0, objectValue, FlowStateObjectGroup.FLOW_STATE_GROUP_RIB_OBJECT_NAME, 0, false);
			Assert.assertTrue(impl.writeMessageRecieved(cdapMessage, 1));
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}	
	}
	
	@Test
	public void writeMessageRecieved_NoGroupClass_True()
	{
		ObjectValue objectValue = new ObjectValue();
		FlowStateEncoder fse = new FlowStateEncoder();
		FakeCDAPSessionManager cdapSessionManager = new FakeCDAPSessionManager();
		FakeRIBDaemon rib = new FakeRIBDaemon();
		FlowStateObject obj1 = new FlowStateObject(1,1,2,1, true, 1, 0);
		IPCProcess ipc = new FakeIPCProcess(cdapSessionManager, rib, fse);
		impl.setIPCProcess(ipc);
		impl.setAlgorithm(new DijkstraAlgorithm(), new Vertex(1));
		CDAPMessage cdapMessage = null;

		try
		{
			
			objectValue.setByteval(fse.encode(obj1));
			cdapMessage = cdapSessionManager.getWriteObjectRequestMessage(1, null,
				null, FlowStateObject.FLOW_STATE_RIB_OBJECT_CLASS, 0, objectValue, obj1.getID(), 0, false);
		} 
		catch (Exception e) 
		{
			e.printStackTrace();
		}
		Assert.assertFalse(impl.writeMessageRecieved(cdapMessage, 1));
	}
}
