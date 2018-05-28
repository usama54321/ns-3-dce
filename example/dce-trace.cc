#include <iostream>
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/dce-module.h"

using namespace ns3;

static Ptr<OutputStreamWrapper> cWndStream;

static void
tracer (uint32_t oldval, uint32_t newval)
{
  std::cout << "Oldvalue: " << oldval << " Newvalue: "<< newval << std::endl;
  *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << "," << oldval << "," << newval << std::endl;
}

static void
cwnd (std::string file_name)
{
  std::cout << "cwnd" << std::endl;
  AsciiTraceHelper ascii;
  cWndStream = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&tracer));
}


int main (int argc, char *argv[])
{
  std::cout << "main" << std::endl;
  bool trace = true;
  bool pcap = false;
  double simulation_time = 10.0;
  std::string trace_file = "cwnd_trace_file.trace";
  std::string stack = "linux";
  unsigned int num_flows = 3;
  double start_time = 0.1;
  double stop_time = start_time + 10.0;
  std::string sock_factory = "ns3::TcpSocketFactory";


  CommandLine cmd;
  cmd.AddValue ("trace", "Enable trace", trace);
  cmd.AddValue ("pcap", "Enable PCAP", pcap);
  cmd.AddValue ("time", "Simulation Duration", simulation_time);
  cmd.AddValue ("stack", "Network stack: either ns3 or Linux", stack);
  cmd.Parse (argc, argv);

  if (stack != "ns3" && stack != "linux")
    {
      std::cout << "ERROR: " <<  stack << " is not available" << std::endl; 
    }

  NodeContainer nodes;
  nodes.Create (3);

  PointToPointHelper link;
  link.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  link.SetChannelAttribute ("Delay", StringValue ("0.01ms"));

  NetDeviceContainer rightToRouterDevices, routerToLeftDevices;
  rightToRouterDevices = link.Install (nodes.Get (0), nodes.Get (1));
  routerToLeftDevices = link.Install (nodes.Get (1), nodes.Get (2));

  DceManagerHelper dceManager;
  LinuxStackHelper linuxStack;

  InternetStackHelper internetStack;

  if (stack == "linux")
    {
      
      sock_factory = "ns3::LinuxTcpSocketFactory";
      dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                                  "Library", StringValue ("liblinux.so"));
      linuxStack.Install (nodes);
      dceManager.Install (nodes);
    }
  else
    {
      internetStack.InstallAll ();
    }
  
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer sink_interfaces;  

  Ipv4InterfaceContainer interfaces;
  address.NewNetwork ();
  interfaces = address.Assign (rightToRouterDevices);
  address.NewNetwork ();
  interfaces = address.Assign (routerToLeftDevices);
  sink_interfaces.Add (interfaces.Get (1));  

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  if(stack == "linux")
    {
      std::cout << "linux stack start" << std::endl;
      LinuxStackHelper::PopulateRoutingTables ();
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.conf.default.forwarding", "1");
      //linuxStack.SysctlSet (nodes.Get (2), ".net.ipv4.conf.default.forwarding", "1");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.tcp_congestion_control", "westwood");
      //linuxStack.SysctlSet (nodes.Get (2), ".net.ipv4.tcp_congestion_control", "westwood");
      //
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.tcp_sack", "0");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.tcp_frto", "0");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.tcp_dsack", "0");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.tcp_ecn", "0");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.tcp_fack", "0");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.core.wmem_max", "12582912");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.core.rmem_max", "12582912");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.tcp_rmem", "10240 87380 12582912");
      //linuxStack.SysctlSet (nodes.Get (0), ".net.ipv4.tcp_wmem", "10240 87380 12582912");

      std::cout << "linux stack end" << std::endl;
    }

  uint16_t port = 2000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));

  PacketSinkHelper sinkHelper (sock_factory, sinkLocalAddress);
  std::cout << "packet sink helper" << std::endl;
  
  AddressValue remoteAddress (InetSocketAddress (sink_interfaces.GetAddress (0, 0), port));
  std::cout << "remote address" << std::endl;
  BulkSendHelper sender (sock_factory, Address ());
  std::cout << "bulk send" << std::endl;
    

  sender.SetAttribute ("Remote", remoteAddress);
  ApplicationContainer sendApp = sender.Install (nodes.Get (0));
  sendApp.Start (Seconds(0.0));
  sendApp.Stop (Seconds(10));
  std::cout << "appplication end" << std::endl;

  sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
  ApplicationContainer sinkApp = sinkHelper.Install (nodes.Get (2));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (12));
  std::cout << "sink helper end" << std::endl;

  if (trace)
    {
      std::cout << "trace" << std::endl;
      Simulator::Schedule (Seconds (0.00001), &cwnd, trace_file);
    }
  std::cout << "trace end" << std::endl;

  if (pcap)
    {
       std::cout << "Pcap Enabled" << std::endl;
       link.EnablePcapAll ("cwnd-trace", true);
    }

  Simulator::Stop (Seconds (15));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
