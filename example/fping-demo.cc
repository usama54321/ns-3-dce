/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sourabh Jain <sourabhjain560@outlook.com>
 *         Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

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
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

std::string intToString (int a);
std::string Ipv4AddressToString (Ipv4Address ip);

int main (int argc, char *argv[])
{
  bool pcap = false;
  int simulationTime = 70;
  unsigned int rightCout = 2;
  unsigned int leftCount = 3;
  unsigned int maxBytes = 0; 
  double startTime = 2.0;

  CommandLine cmd;
  cmd.AddValue ("pcap", "Enable PCAP", pcap);
  cmd.AddValue ("time", "Simulation Duration", simulationTime);
  cmd.Parse (argc, argv);

  if (simulationTime < 5)
    {
      std::cout << "WARNING: " << "Simluation time sholud be greater than 5 seconds" << std::endl;
    }


  PointToPointHelper p1, p2;
  p1.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p1.SetChannelAttribute ("Delay", StringValue ("5ms"));
  
  p2.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2.SetChannelAttribute ("Delay", StringValue ("100ms"));
  PointToPointDumbbellHelper dumbbellHelper(leftCount, p1, rightCout, p1, p2);
  
  InternetStackHelper stack;
  dumbbellHelper.InstallStack (stack);

  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute ("FiberManagerType", StringValue ("UcontextFiberManager"));
  
  for (int i = 0; i < dumbbellHelper.LeftCount (); i++)
    {
      dceManager.Install (dumbbellHelper.GetLeft (i));
    }

  for (int i = 0; i < dumbbellHelper.RightCount (); i++)
    {
      dceManager.Install (dumbbellHelper.GetRight (i));
    }

  dumbbellHelper.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
              Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
              Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));


  Config::Set ("/$ns3::NodeListPriv/NodeList/"+intToString(dumbbellHelper.GetLeft ()->GetId ())+"/$ns3::Node/$ns3::TrafficControlLayer/RootQueueDiscList/0/$ns3::FifoQueueDisc/MaxSize", QueueSizeValue (QueueSize ("3000p")));

Config::Set ("/$ns3::NodeListPriv/NodeList/"+intToString(dumbbellHelper.GetRight ()->GetId ())+"/$ns3::Node/$ns3::TrafficControlLayer/RootQueueDiscList/0/$ns3::FifoQueueDisc/MaxSize", QueueSizeValue (QueueSize ("3000p")));
  uint16_t port = 9000;

  ApplicationContainer sourceApps, sinkApps;
  
  BulkSendHelper sourceBulkSend ("ns3::TcpSocketFactory",
                         InetSocketAddress (dumbbellHelper.GetLeftIpv4Address (1), port));
  sourceBulkSend.SetAttribute ("MaxBytes", UintegerValue (maxBytes));

  OnOffHelper sourceOnOffSend ("ns3::TcpSocketFactory", Address ());
  sourceOnOffSend.SetAttribute ("OnTime", StringValue ("ns3::UniformRandomVariable[Min=0.|Max=1.]"));
  sourceOnOffSend.SetAttribute ("OffTime", StringValue ("ns3::UniformRandomVariable[Min=0.|Max=1.]"));
  AddressValue remoteAddress (InetSocketAddress (dumbbellHelper.GetLeftIpv4Address (2), port));
  sourceOnOffSend.SetAttribute ("Remote", remoteAddress);

  sourceApps.Add (sourceBulkSend.Install (dumbbellHelper.GetRight (1))); 
  sourceApps.Add (sourceOnOffSend.Install (dumbbellHelper.GetRight (1)));

  sourceApps.Start (Seconds (startTime));
  sourceApps.Stop (Seconds (simulationTime - 2));

  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  sinkApps.Add (sink.Install (dumbbellHelper.GetLeft (1)));
  sinkApps.Add (sink.Install (dumbbellHelper.GetLeft (2)));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (simulationTime));

  DceApplicationHelper dce;
  ApplicationContainer dceApps;
  dce.SetStackSize (1 << 20);
  dce.SetBinary ("fping");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArguments ("-n", "-D", "-C");
  dce.AddArguments (intToString (simulationTime * 5));
  dce.AddArguments ("-p", "200", "-t");
  dce.AddArguments (intToString(simulationTime*2*1000));

  for (int i = 0; i < dumbbellHelper.RightCount (); i++)
    {
      dce.AddArguments (Ipv4AddressToString(dumbbellHelper.GetRightIpv4Address(i)));
    }

  dce.AddArgument (Ipv4AddressToString(dumbbellHelper.GetLeftIpv4Address (1)));
  dceApps = dce.Install (dumbbellHelper.GetLeft (0));
  dceApps.Start (Seconds (startTime));
  dceApps.Stop (Seconds (simulationTime));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  if (pcap)
  {
     p1.EnablePcapAll ("fping-demo", false);
  }

  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

std::string Ipv4AddressToString (Ipv4Address ip)
{
  std::ostringstream ipStream;
  ip.Print (ipStream);
  return ipStream.str();
}

std::string intToString (int val)
{
  std::stringstream ss;
  ss << val;
  return ss.str ();
}

