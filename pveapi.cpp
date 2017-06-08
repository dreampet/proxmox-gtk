#include "pveapi.h"
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

PVEAPI::PVEAPI ()
{
    curl = new curlite::Easy();
    headers = new curlite::List();
}

PVEAPI::~PVEAPI()
{
  delete curl;
  delete headers;
}

bool PVEAPI::Authentication (PVE_LOGIN *login)
{
  SetProxmox( login->get("server").toString() );
  std::string params;
  params += "username=";
  params += login->get("user").toString().c_str();
  params += "&password=";
  params += login->get("pass").toString().c_str();
  params += "&realm=";
  params += login->get("realm").toString().c_str();

  Jzon::Node node = Request( Format(PVEAPI_GET_TICKET), params );
  if (!node.isNull())
  {
    m_ticket = node.get("ticket").toString();
    m_csrf = node.get("CSRFPreventionToken").toString();
    return true;
  }
  return false;
}

PVEAPI_VM_STATUS PVEAPI::Status(PVE_VM *vm)
{
  Jzon::Node node = Request( Format(PVEAPI_GET_STATUS, vm->get("node").toString(), vm->get("id").toInt() ) );
  if (!node.isNull())
  {
    if (node.get("status").toString() == "running")
      return VM_RUNNING;
    else if (node.get("status").toString() == "stopped")
      return VM_STOPPED;
    else
      return VM_UNKNOWN;
  }
  return VM_INVALID;
}

bool PVEAPI::compare_node (const PVE_NODE * first, const PVE_NODE * second)
{
  return first->get("name").toString() < second->get("nake").toString();
}
/*
  get the node list
  caller should delete the ret after used it.
*/
std::list <PVE_NODE *> PVEAPI::Nodes()
{
  std::list <PVE_NODE *> ret;
  Jzon::Node node = Request( Format(PVEAPI_GET_NODES) );
  if (!node.isNull())
  {
    for (int i = 0; i < node.getCount(); i++)
    {
      PVE_NODE *n = new PVE_NODE(Jzon::Node::T_OBJECT);
      n->add ("name", node.get(i).get("node") );
      ret.push_back( n );
    }
  }
  ret.sort(compare_node);
  return ret;
}

bool PVEAPI::compare_vm(const PVE_VM * first, const PVE_VM * second)
{
  return first->get("id").toInt() < second->get("id").toInt();
}
/*
  get the VM list
  caller should delete the ret after used it.
*/
std::list <PVE_VM *> PVEAPI::Vms(PVE_NODE *node)
{
  std::list <PVE_VM *> ret;
  Jzon::Node n = Request( Format(PVEAPI_GET_VMS, node->get("name").toString()) );
  if (!n.isNull())
  {
    for (int i = 0; i < n.getCount(); i++)
    {
      PVE_VM *vm = new PVE_VM (Jzon::Node::T_OBJECT);
      vm->add( "id", n.get(i).get("vmid") );
      vm->add( "name", n.get(i).get("name") );
      vm->add( "node", node->get("name") );
      
      ret.push_back(vm);
    }
  }
  ret.sort(compare_vm);
  return ret;
}

/*
  start a vm
*/
void PVEAPI::Start( PVE_VM *vm )
{
   Request ( Format(PVEAPI_VM_START, vm->get("name").toString(), vm->get("id").toInt()), "", true, METHOD_POST);
}

/*
  stop a running vm
*/
void PVEAPI::Stop( PVE_VM *vm )
{
   Request ( Format(PVEAPI_VM_STOP, vm->get("name").toString(), vm->get("id").toInt()), "", true, METHOD_POST);
}

/*
  shutdown a running vm
*/
void PVEAPI::Shutdown ( PVE_VM *vm, int timeout )
{
   Request ( Format(PVEAPI_VM_SHUTDOWN, vm->get("name").toString(), vm->get("id").toInt()), "", true, METHOD_POST);
}

/*
  reset a running vm
*/
void PVEAPI::Reset(PVE_VM *vm)
{
   Request ( Format(PVEAPI_VM_RESET, vm->get("name").toString(), vm->get("id").toInt()), "", true, METHOD_POST);
}

/*
  execute an extenal spice program to connect
*/
void PVEAPI::Console(PVE_VM *vm)
{
  std::string proxy;
  proxy += "proxy=";
  proxy += m_proxmox.c_str();
  Jzon::Node node = Request (Format(PVEAPI_VM_CONSOLE, vm->get("name").toString(), vm->get("id").toInt()), proxy, true);
  if (!node.isNull())
  {
    std::ofstream tmpfile( "/tmp/spiceproxy" );
  
    tmpfile << "[virt-viewer]" << "\n";
    tmpfile << "delete-this-file=" << node.get("delete-this-file").toInt() << "\n";
    tmpfile << "ca=" << node.get("ca").toString() << "\n";
    tmpfile << "host=" << node.get("host").toString() << "\n";
    tmpfile << "release-cursor=" << node.get("release-cursor").toString() << "\n";
    tmpfile << "toggle-fullscreen=" << node.get("toggle-fullscreen").toString() << "\n";
    tmpfile << "tls-port=" << node.get("tls-port").toInt() << "\n";
    tmpfile << "host-subject=" << node.get("host-subject").toString() << "\n";
    tmpfile << "secure-attention=" << node.get("secure-attention").toString() << "\n";
    tmpfile << "title=" << node.get("title").toString() << "\n";
    tmpfile << "proxy=" << node.get("proxy").toString() << "\n";
    tmpfile << "password=" << node.get("password").toString() << "\n";
    tmpfile << "type=" << node.get("type").toString() << "\n";
    tmpfile << "fullscreen=1" << "\n";
    tmpfile << "disable-effects=all" << "\n";
    tmpfile << "enable-usbredir=1" << "\n";
    tmpfile << "enable-usb-autoshare=1" << "\n";
    tmpfile.close();

    std::string cmd = "/usr/bin/remote-viewer ";
    cmd += "/tmp/spiceproxy";
    if(fork() == 0){
      std::system ( cmd.c_str() );
      exit(0);
    }
  }
}

std::string PVEAPI::Format(std::string path, std::string name, int id)
{
  std::string needle;
  size_t pos;

  needle = "{node}";
  pos = path.find( needle );
  if (pos != std::string::npos)
  {
    path.erase( pos, needle.length());
    path.insert( pos, name );
  }

  needle = "{vmid}";
  pos = path.find( needle );
  if (pos != std::string::npos)
  {
    path.erase( pos, needle.length());
    path.insert( pos, std::to_string(id));
  }

  return path;
}

void PVEAPI::SetProxmox( std::string _proxmox )
{
  m_proxmox = _proxmox;
}

/*
  perform a query, return the json node
*/
Jzon::Node PVEAPI::Request(const std::string path, const std::string params, bool withtoken, PVEAPI_METHOD method)
{
  curl->reset();
  curl->set( CURLOPT_SSL_VERIFYPEER, 0L );
  curl->set( CURLOPT_SSL_VERIFYHOST, 0L );
  curl->set( CURLOPT_VERBOSE, 0L );
  // set timeout to 5 seconds
  curl->set( CURLOPT_TIMEOUT, 5L );
  
  std::string url;
  url += "https://";
  url += m_proxmox.c_str();
  url += ":";
  url += std::to_string(8006).c_str();
  url += path.c_str();

  curl->set ( CURLOPT_URL, url.c_str() );
  
  if ( !params.empty() )
  {
    curl->set ( CURLOPT_POSTFIELDS, params.c_str() );
    curl->set ( CURLOPT_POSTFIELDSIZE, (long)params.length() );
  }

  if ( !m_ticket.empty() )
  {
    std::string cookie("PVEAuthCookie=");
    cookie += m_ticket.c_str();
    curl->set ( CURLOPT_COOKIE, cookie.c_str() );
  }

  if ( withtoken && !m_csrf.empty() )
  {
    headers->release();
    std::string header = "CSRFPreventionToken: ";
    header += m_csrf.c_str();
    headers->append(header.c_str());
    curl->set ( CURLOPT_HTTPHEADER, headers->get() );
  }

  if (method != METHOD_AUTO)
  {
    switch (method)
    {
      case METHOD_GET: 
        curl->set ( CURLOPT_HTTPGET, 1L );
        break;
      case METHOD_PUT: 
        curl->set ( CURLOPT_PUT, 1L );
        break;
      case METHOD_POST: 
        curl->set ( CURLOPT_POST, 1L );
        break;
    }
  }

  std::ostringstream oss;
  curl->onWrite( [&oss] (char *data, size_t size, size_t n, void *) -> size_t
  {
    oss.write( data, size * n );
    return oss.good() ? size * n : -1;
  } );

  try {
    if (!curl->perform())
    {
      std::cerr << "perform error!" << std::endl;
    }
  } catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }

  oss.flush();

  Jzon::Parser parser;
  Jzon::Node ret = parser.parseString(oss.str());
  if (!ret.isValid())
  {
    std::cerr << parser.getError() << std::endl;
  }

  if (!ret.isNull())
    return ret.get("data");
  else
    return ret;
}
