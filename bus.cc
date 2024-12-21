#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BusTopology");

int main() {
    uint32_t nNodes = 4; 
    uint16_t port = 50000; 

    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(137));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("14kb/s"));

    NodeContainer nodes;
    nodes.Create(nNodes);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper address;
    Ipv4InterfaceContainer interfaces;
    for (uint32_t i = 0; i < nNodes - 1; i++) {
        NetDeviceContainer devices = pointToPoint.Install(nodes.Get(i), nodes.Get(i + 1));
        address.SetBase(("10.1." + std::to_string(i + 1) + ".0").c_str(), "255.255.255.0");
        interfaces.Add(address.Assign(devices));
    }

    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(nNodes - 1));
    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(10.0));

    OnOffHelper onOffHelper("ns3::TcpSocketFactory", 
                            InetSocketAddress(interfaces.GetAddress(nNodes - 2), port));
    onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    ApplicationContainer sourceApp = onOffHelper.Install(nodes.Get(0));
    sourceApp.Start(Seconds(1.0));
    sourceApp.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    pointToPoint.EnablePcapAll("bus-topology");

    AnimationInterface anim("bus-topology.xml");
    double xPos = 50.0, yPos = 250.0, xDelta = 150.0;
    for (uint32_t i = 0; i < nNodes; ++i) {
        anim.SetConstantPosition(nodes.Get(i), xPos + i * xDelta, yPos);
    }

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
